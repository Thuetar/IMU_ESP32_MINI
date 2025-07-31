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

bool start_overseer_webserver();
bool oc_configure_i2c_hardware();
std::vector<int> get_i2c_device_list();
void overseer_led_indicator_configure();
void overseer_led_start_sequence();
#endif
