#ifndef MAIN_H
#define MAIN_H

#include "build_config.h"
#include <Arduino.h>
#include <SPI.h>
#include "FS.h"
#include "SPIFFS.h"
#include "config/ConfigManager.h"
#include "net/WifiManager.h"
#include "web/WebServerManager.h"
#include "api/IMUApi.h"
#include "device/ads/MPLEX.h"
#include "device/environment/DHTFAMILY_instance.h"
//#include "device/environment/DHTFAMILY_instance.h"
namespace overseer::client::core::string_table {

    constexpr auto message_header = "******************************************";
    constexpr auto message_header__load_message = "***         IMU CLIENT POC / WIP            ***";
    constexpr auto message_boot__preload_init = "***  SYSTEM INIT  ***";
    constexpr auto message_boot__preload_sytemconfig_load = "Loading SPIFFS File System ...";
    constexpr auto message_boot__system_config_load = "Loading Configuration ";
    //constexpr auto message_boot__system_i2c_init = F("******************************************" CR);
    //constexpr auto message_boot__system_imu_load = F("******************************************" CR);
    //constexpr auto message_boot__user_dc1__load = F("******************************************" CR);
    //constexpr auto message_boot__system_ac1_load = F("******************************************" CR);
    constexpr auto message_boot__system_wifi_init = "Initializing Wifi";

};

bool start_overseer_webserver();
bool oc_configure_i2c_hardware();
std::vector<int> get_i2c_device_list();
void overseer_led_indicator_configure();
void overseer_led_start_sequence();
#endif



