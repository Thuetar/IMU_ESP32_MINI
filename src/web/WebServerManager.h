//WebServerManager.h
#pragma once
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include "../config/ConfigManager.h"
#include <vector>
#include "api/DeviceApi.h"
#include "system/SystemMonitor.h"
class WebServerManager {
public:
    WebServerManager(config::ConfigManager& cfg);
    //WebServerManager(config::ConfigManager& cfg, system_utils::SystemMonitor*);
    void begin();
    void broadcast();
    void registerDeviceApi(api::DeviceApi* deviceApi);
    void setSystemMonitor(system_utils::SystemMonitor* monitor);
    AsyncWebServer& getServer() { return server; }

private:
    AsyncWebServer server;
    config::ConfigManager& config;
    system_utils::SystemMonitor* sysMon;
    std::vector<api::DeviceApi*> deviceApis;
    
    void setupRoutes();
};




