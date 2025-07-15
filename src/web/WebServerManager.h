#pragma once
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include "../config/ConfigManager.h"
#include <vector>
#include "api/DeviceApi.h"

class WebServerManager {
public:
    WebServerManager(config::ConfigManager& cfg);
    void begin();
    void broadcast();
    void registerDeviceApi(api::DeviceApi* deviceApi);
    AsyncWebServer& getServer() { return server; }

private:
    AsyncWebServer server;
    config::ConfigManager& config;
    std::vector<api::DeviceApi*> deviceApis;
    
    void setupRoutes();
};
