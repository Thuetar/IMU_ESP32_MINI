//main.cpp
/*
    IMU_ESP32 MINI
*/
#include "main.h"
#include "system_config.h"
#include "cli/CommandProcessor.h" 
#include "devices/IMU/MPU6000/MPU6000.h"    
#include "devices/IMU/MPU6000/MPU6000_instance.h"
#include "system/SystemMonitor.h"

using namespace overseer;

system_utils::SystemMonitor* sysMon = nullptr;
RUNNING_CONFIG running_config;
config::ConfigManager configManager(SPIFFS);
WiFiManager* wifi = nullptr;
WebServerManager* web = nullptr;
String inputBuffer;
CommandProcessor commandProcessor;

void setup() {

  Serial.begin(115200);
  while (!Serial && !Serial.available()) {}
  
  Log.begin   (LOG_LEVEL_VERBOSE, &Serial);  
  Log.notice(F(CR "******************************************" CR)); // Info string with Newline                                                                    
  Log.notice("***         IMU CLIENT POC / WIP            ***" CR); // Info string in flash memory
  Log.notice(F(CR "******************************************" CR)); 
  
  Log.verbose(F(CR "Loading SPIFFS File System ..." CR));
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
  }
  Log.verbose(F(CR "Loading Configuration " CR));
  if (!configManager.begin()) {
        Log.errorln(F("Config load failed"));
        while (true);
  }
  
  
  Log.notice(F(CR "******************************************" CR)); 
  
  Log.infoln("\t\t Starting System Monitor");
  sysMon = system_utils::createSystemMonitor(configManager);
  sysMon->begin();
  
  
  
  //Log.infoln("\t\t Initializing I2C Bus");
  //I2CCore i2Ccore;
  //i2Ccore.i2c_bus_reset(running_config.hardware_config.imu.pins.sda, running_config.hardware_config.imu.pins.scl);
  //delay(100);
  //Log.infoln("\t\t Initializing I2C Bus");
  //i2Ccore.address_scan();

  Log.infoln("\t\t Initializing Wifi");
  wifi = new WiFiManager(configManager);
  wifi->begin();

  if (!wifi->isConnected()) {
      Log.errorln(F("WiFi connection failed. Halting."));
      while (true);
  }
  
  Log.infoln("\t\t IMU Device");
  Log.infoln("enable_imu_print::%s", running_config.debug_options.enable_imu_print ? "true" : "false"); 
  //Log.infoln("enable_imu_print: %s", RUNNING_CONFIG::DEBUG_OPTIONS::enable_imu_print ? "true" : "false");
  //Log.infoln("enable_imu_print: %s", RUNNING_CONFIG::DEBUG_OPTIONS::enable_imu_print ? "true" : "false");

  Log.infoln("imu_log_message_interval::%u", running_config.debug_options.imu_log_message_interval); 
  Log.infoln("smoothing_alpha:: %F", running_config.hardware_config.imu.filter_config.smoothing_alpha); 
  Log.infoln("spike_threshold:: %F", running_config.hardware_config.imu.filter_config.spike_threshold); 
  Log.infoln("window_smoothing_alpha:: %F", running_config.hardware_config.imu.filter_config.window_smoothing_alpha); 
  Log.infoln("PINS SDA:: %d", running_config.hardware_config.imu.pins.sda); 
  Log.infoln("PINS SCL:: %d", running_config.hardware_config.imu.pins.scl); 

  Log.infoln("Initializing IMU Device");
  running_config.hardware_config.imu.mpu = &overseer::device::imu::getInstance();
  Log.infoln("Starting IMU");
  if (running_config.hardware_config.imu.mpu) {
      running_config.hardware_config.imu.mpu->begin();
  }
  
  //Set the Logging Level
  Log.setLevel(configManager.getLogLevel()); 

  Log.infoln("Loading Web Server");  
  web = new WebServerManager(configManager);  
  auto* imuApi = new overseer::device::api::IMUApi(web->getServer(), *running_config.hardware_config.imu.mpu, configManager);
  web->setSystemMonitor(sysMon);
  web->registerDeviceApi(imuApi);
  web->begin();
  
  Serial.println("READY");
}

void loop() {  
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
  
  if (sysMon) sysMon->update();
  
  if (running_config.hardware_config.imu.enable_imu == true) {
    Log.verboseln("Doing IMU Update...");
    running_config.hardware_config.imu.mpu->update();    

    Log.verboseln("Fetching IMU Data...");
    auto sensor_data = running_config.hardware_config.imu.mpu->getData();  
    
    Log.verboseln("Smoothing Data...");
    running_config.hardware_config.imu.mpu->smoothAndFilterMPUData(sensor_data);
    
    if (running_config.debug_enable == true && running_config.debug_options.enable_imu_print == true) {       
      if (currentMillis - running_config.debug_options.imu_log_last_print_time >= running_config.debug_options.imu_log_message_interval) {
          running_config.debug_options.imu_log_last_print_time = currentMillis;          
          Log.verboseln("Print Data...");
          running_config.hardware_config.imu.mpu->printMPUData (sensor_data); 
      } 
    }
  
    Log.verboseln("Done with IMU");
    
  } //end if
  
  //if (web) {
  //  web->broadcast();  // assuming WebServerManager exposes IMUApi loop()
  //}
  yield();
  
}

