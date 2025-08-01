//main.cpp
/*
    IMU_ESP32 MINI
*/
#include "main.h"
#include "system_config.h"
#include "cli/CommandProcessor.h" 
#include "device/ads/MPLEX.h"
#include "device/IMU/MPU6000/MPU6000_instance.h"
#include "api/MPLEXApi.h"

#include "system/SystemMonitor.h"
#include "Arduino.h"

using namespace overseer;
using namespace overseer::device::ads;

system_utils::SystemMonitor* sysMon = nullptr;
RUNNING_CONFIG running_config;
config::ConfigManager configManager(SPIFFS);
WiFiManager* wifi = nullptr;
WebServerManager* web = nullptr;
String inputBuffer;
CommandProcessor commandProcessor;
MPLEX mplex;


void led_blink(bool mode ) {  
  digitalWrite(STATUS_LED_RED, mode); // LED ON
  delay(50);
  digitalWrite(STATUS_LED_YELLOW, mode); // LED ON
  delay(25);
  digitalWrite(STATUS_LED_GREEN, mode); // LED Off  
  sleep(5);
}

void set_system_failed (String message) {
  Serial.println(message);
  Serial.flush();
  while (true) {
    digitalWrite(STATUS_LED_RED, HIGH); // LED ON
    delay(100);
    digitalWrite(STATUS_LED_RED, LOW); // LED ON
    delay(250);
    digitalWrite(STATUS_LED_RED, HIGH); // LED ON
    delay(100);
    digitalWrite(STATUS_LED_RED, LOW); // LED ON
    delay(100);
    sleep(5);
    yield();
  }
}

void blink_blue_led()
{
  digitalWrite(STATUS_LED_INDICATOR, HIGH); // Blue LED ON
  delay(250);
  digitalWrite(STATUS_LED_INDICATOR, LOW); // Blue LED off
  delay(500);
  digitalWrite(STATUS_LED_INDICATOR, HIGH); // Blue LED ON
  delay(250);
  digitalWrite(STATUS_LED_INDICATOR, LOW); // Blue LED off
}

void blink_green_led()
{
  digitalWrite(STATUS_LED_GREEN, HIGH); // Blue LED ON
  delay(250);
  digitalWrite(STATUS_LED_GREEN, LOW); // Blue LED off
  delay(500);
  digitalWrite(STATUS_LED_GREEN, HIGH); // Blue LED ON
  delay(250);
  digitalWrite(STATUS_LED_GREEN, LOW); // Blue LED off
}

bool oc_configure_i2c_hardware()
{
  bool ret = false;
  Log.infoln("\t ___ Wire (i2c) Starting ___");
  Log.info("PINS SDA:: %d, \t", OVERSEER_SDA_PIN); Log.infoln("PINS SCL:: %d", OVERSEER_SCL_PIN);

  ret = Wire.begin( OVERSEER_SDA_PIN, OVERSEER_SCL_PIN);
  Log.infoln ("Bret: %b", ret);
  
  if (ret == false) {
    Log.infoln ("Bret is false");
    return false;    
  }
  
  if (mplex.begin() == false) {
    Log.infoln ("MPLEX is False");
    return false;    
  }
  else {
    Log.verboseln(F("MPLEX (ADS1115) Started"));
  }
  
  Log.infoln("\t\t Starting DC Current Sensor");
  mplex.calibrateAllChannels();
  Log.infoln("MPLEX::All channels calibrated");


  return true;
}

bool start_overseer_webserver()
{
  Log.infoln("Web Server:: Starting");
  web = new WebServerManager(configManager);
  auto *imuApi = new overseer::device::api::IMUApi(web->getServer(), *running_config.hardware_config.imu.mpu, configManager);
  auto *mplexApi = new overseer::device::api::MPLEXApi(web->getServer(), mplex, configManager);
  web->setSystemMonitor(sysMon);
  web->registerDeviceApi(imuApi);
  Log.infoln("Web Server:: Registering MultiPlexer API");
  web->registerDeviceApi(mplexApi);

  web->begin();
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  pinMode(STATUS_LED_INDICATOR, OUTPUT);
  blink_blue_led();
  Serial.flush();
  yield();
  digitalWrite(STATUS_LED_GREEN, HIGH); // Blue LED ON
  sleep(1);

  Serial.println("Setup!!");
  //while (!Serial && !Serial.available()) {}
  //while (!Serial) {}
  Serial.flush();
  
  Log.begin   (LOG_LEVEL_VERBOSE, &Serial);  
  Log.notice(F(CR "******************************************" CR)); // Info string with Newline                                                                    
  Log.notice("***         IMU CLIENT POC / WIP            ***" CR); // Info string in flash memory
  Log.notice(F(CR "******************************************" CR)); 
  Serial.flush();

  Log.verbose(F(CR "Loading SPIFFS File System ..." CR));
  Serial.flush();
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        Serial.flush();
        set_system_failed("SPIFFS Mount Failed");
        return;
  }
  blink_green_led();
  Serial.flush();

  Log.verbose(F(CR "Loading Configuration " CR));
  Serial.flush();
  if (!configManager.begin()) {        
        set_system_failed(F("PANIC!! Config load failed"));        
  }

  //Set the Logging Level
  //Log.setLevel(configManager.getLogLevel());

  Log.notice(F(CR "******************************************" CR)); 
  Log.infoln("\t\t Starting System Monitor");
  sysMon = system_utils::createSystemMonitor(configManager);
  sysMon->begin();

  Log.infoln("\t\t Initializing Wifi");
  wifi = new WiFiManager(configManager);
  wifi->begin();

  if (!wifi->isConnected()) {      
      set_system_failed(F("WiFi connection failed. Halting."));      
  }
  Serial.println("\n\n AM I HERE?? \n\n");
  Log.notice(F(CR "******************************************" CR)); 
  
  Log.infoln("\t\t Starting User Hardware Setup");  
  pinMode(STATUS_LED_GREEN, OUTPUT);
  pinMode(STATUS_LED_YELLOW, OUTPUT);
  pinMode(STATUS_LED_RED, OUTPUT);  
  pinMode(STATUS_LED_INDICATOR, OUTPUT);  

  digitalWrite(STATUS_LED_GREEN, LOW); // LED OFF
  digitalWrite(STATUS_LED_YELLOW, LOW); // LED OFF
  digitalWrite(STATUS_LED_RED, LOW); // LED OFF
  digitalWrite(STATUS_LED_INDICATOR, HIGH); // Blue LED ON
  
  Log.infoln("Configure and Start I2C.");  
  if (oc_configure_i2c_hardware() == false) {    
    Log.errorln("i2c startup failed!!");
    mplex.scan_i2c_bus ();
    set_system_failed(F("MPLEX Failed Starting!"));
  }

  led_blink (HIGH);
  delay(500);
  led_blink (LOW);

  Log.infoln("\t\t IMU Device");
  Log.infoln("enable_imu_print::%s", running_config.debug_options.enable_imu_print ? "true" : "false"); 
  //Log.infoln("enable_imu_print: %s", RUNNING_CONFIG::DEBUG_OPTIONS::enable_imu_print ? "true" : "false");
  //Log.infoln("enable_imu_print: %s", RUNNING_CONFIG::DEBUG_OPTIONS::enable_imu_print ? "true" : "false");

  Log.infoln("imu_log_message_interval::%u", running_config.debug_options.imu_log_message_interval); 
  Log.infoln("smoothing_alpha:: %F", running_config.hardware_config.imu.filter_config.smoothing_alpha); 
  Log.infoln("spike_threshold:: %F", running_config.hardware_config.imu.filter_config.spike_threshold); 
  Log.infoln("window_smoothing_alpha:: %F", running_config.hardware_config.imu.filter_config.window_smoothing_alpha); 
  
  /*
    TODO: Check that we have the right i2c devices...
  */
  Log.infoln("Initializing IMU Device");
  running_config.hardware_config.imu.mpu = &overseer::device::imu::getInstance();
  Log.infoln("Starting IMU");
  if (running_config.hardware_config.imu.mpu) {
      running_config.hardware_config.imu.mpu->begin();
  }
  
  
  // Start App (web) Server
  if (start_overseer_webserver() == false) {
    Log.errorln(F("Webserver Failed to Start"));
    set_system_failed(F("Webserver Failed to Start"));
  } else {
    Log.infoln ("Web Started");
  }

  Serial.println("READY");
}



void loop ()
{
  unsigned long currentMillis = millis(); //For timing of things :)

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
        if (inputBuffer.length() > 0) {
            commandProcessor.processLine(inputBuffer);
            inputBuffer = "";
        }
    } else {
        inputBuffer += c;
    }
  }
  
  if (currentMillis - running_config.debug_options.log_last_print_time >= running_config.debug_options.log_message_interval) {
    if (sysMon) sysMon->update();
  }
  
  /**
   * @details Do MultiPlexer Readings... 
   * 
   * Note:: There are two Las_Log times. One for IMU and Another for Device MPLEX
   */    
  auto channelData = mplex.getChannelData(WCS_SENSOR_1_ADS_CHANNEL);
  if (currentMillis - running_config.debug_options.log_last_print_time >= running_config.debug_options.log_message_interval) {
      running_config.debug_options.log_last_print_time = currentMillis;  
      if (running_config.debug_enable == true) {       
        Serial.print("ADS:0 Raw Reading:: "); Serial.println(channelData.raw_value);         
        Log.verboseln("Print mplex/ADS Data...");
        Serial.print("\tAnalog voltage 0: "); Serial.print(channelData.voltage); Serial.print('\n'); 
        Serial.print("\tScaled value 0: "); Serial.print(channelData.scaled_value); Serial.print('\n'); 
        Serial.print("\tChannel valid: "); Serial.print(channelData.valid ? "true" : "false"); Serial.print('\n');
      }      
  }  

  /**
   * @details IMU 
   */  
  running_config.hardware_config.imu.mpu->update();          
  auto sensor_data = running_config.hardware_config.imu.mpu->getData();    
  running_config.hardware_config.imu.mpu->smoothAndFilterMPUData(sensor_data);    

  if (running_config.debug_enable == true) {
    if (running_config.debug_enable == true && running_config.debug_options.enable_imu_print == true) {       
      if (currentMillis - running_config.debug_options.imu_log_last_print_time >= running_config.debug_options.imu_log_message_interval) {
          running_config.debug_options.imu_log_last_print_time = currentMillis;          
          Log.verboseln("Print IMU Data...");
          running_config.hardware_config.imu.mpu->printMPUData (sensor_data);           
      }     
    }  
    Log.verboseln("Done with IMU");
  } //end IMU IF


  yield();
}

