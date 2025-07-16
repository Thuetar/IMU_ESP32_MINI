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
  
  Log.verbose(F(CR "File System ..." CR));
  
  Log.notice(F(CR "******************************************" CR)); 
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
  if (!configManager.begin()) {
        Log.errorln(F("Config load failed"));
        while (true);
  }
  Log.infoln("\t\t Starting System Monitor");
  sysMon = system_utils::createSystemMonitor(configManager);
  sysMon->begin();
  
  wifi = new WiFiManager(configManager);
  wifi->begin();

  if (!wifi->isConnected()) {
      Log.errorln(F("WiFi connection failed. Halting."));
      while (true);
  }
  
  
  Log.infoln("\t\t IMU Device");
  //Log.info("enable_imu_print::%s", running_config.debug_options.enable_imu_print); 
  Log.infoln("imu_log_message_interval::%u", running_config.debug_options.imu_log_message_interval); 
  Log.infoln("smoothing_alpha:: %F", running_config.hardware_config.imu.filter_config.smoothing_alpha); 
  Log.infoln("spike_threshold:: %F", running_config.hardware_config.imu.filter_config.spike_threshold); 
  Log.infoln("window_smoothing_alpha:: %F", running_config.hardware_config.imu.filter_config.window_smoothing_alpha); 
  Log.info("PINS SDA:: %d", running_config.hardware_config.imu.pins.sda); 
  Log.info("PINS SCL:: %d", running_config.hardware_config.imu.pins.scl); 

  Log.infoln("Initializing IMU Device");
  running_config.hardware_config.imu.mpu = &mpu6000::getInstance();
  if (running_config.hardware_config.imu.mpu) {
      running_config.hardware_config.imu.mpu->init();
  }
  
  //Set the Logging Level
  Log.setLevel(configManager.getLogLevel()); 

  Log.infoln("Loading Web Server");  
  web = new WebServerManager(configManager);
  auto* imuApi = new mpu6000::IMUApi(web->getServer(), *running_config.hardware_config.imu.mpu, configManager);
  web->setSystemMonitor(sysMon);
  web->registerDeviceApi(imuApi);
  web->begin();
  
  Serial.println("READY");
}

void loop() {
  if (sysMon) sysMon->update();
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

  if (running_config.hardware_config.imu.enable_imu == true) {
    //Log.verboseln("Doing IMU Update...");
    running_config.hardware_config.imu.mpu->update();    

    //Log.verboseln("Fetching IMU Data...");
    auto sensor_data = running_config.hardware_config.imu.mpu->getData();  
    
    //Log.verboseln("Smoothing Data...");
    running_config.hardware_config.imu.mpu->smoothAndFilterMPUData(
      sensor_data, 
      running_config.hardware_config.imu.filter_config.smoothing_alpha, 
      running_config.hardware_config.imu.filter_config.spike_threshold);
    
    if (running_config.debug_enable == true && running_config.debug_options.enable_imu_print == true) {       
      if (currentMillis - running_config.debug_options.imu_log_last_print_time >= running_config.debug_options.imu_log_message_interval) {
          running_config.debug_options.imu_log_last_print_time = currentMillis;          
          Log.verboseln("Print Data...");
          running_config.hardware_config.imu.mpu->printMPUData (sensor_data); 
      } 
    } 
  
    //Log.verboseln("Done with IMU");
  }//IMU
  
  if (web) {
  //  web->broadcast();  // assuming WebServerManager exposes IMUApi loop()
  }
}

