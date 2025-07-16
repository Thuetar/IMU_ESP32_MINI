
//WebServerManager.cpp
#include "WebServerManager.h"
using namespace config;
//using namespace overseer;

WebServerManager::WebServerManager(ConfigManager& cfg)
  : server(80), config(cfg), sysMon(nullptr) {}

//WebServerManager::WebServerManager(ConfigManager& cfg)
//  : server(80), config(cfg) {}

void WebServerManager::begin() {
    Log.traceln(F("Starting web server..."));
    for (auto* api : deviceApis) {
        api->begin();
    }

    setupRoutes();
    server.begin();
    Log.infoln(F("Web server started on port 80"));
}


void WebServerManager::broadcast() {
    for (auto* api : deviceApis) {
        api->broadcast();
    }
}

void WebServerManager::registerDeviceApi(api::DeviceApi* deviceApi) {
    deviceApis.push_back(deviceApi);
}

void WebServerManager::setSystemMonitor(system_utils::SystemMonitor* monitor) {
    this->sysMon = monitor;
}

void WebServerManager::setupRoutes() {
    // Serve static files
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    server.serveStatic("/", SPIFFS, "/");

    // Config JSON
    server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        StaticJsonDocument<512> doc;
        #pragma GCC diagnostic pop
        
        doc["device"] = config.getString("Device", "name");
        doc["version"] = config.getVersion();
        doc["debug"] = config.getBool("Device", "debug", false);
        doc["wifi_ssid"] = config.getString("WiFi", "ssid");
        
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    if (sysMon) {
        server.on("/api/system", HTTP_GET, [this](AsyncWebServerRequest *request) {
            JsonDocument doc;
            doc["free_heap"] = sysMon->getFreeHeap();
            doc["stack_high_watermark"] = sysMon->getStackHighWaterMark();

            String json;
            serializeJson(doc, json);
            request->send(200, "application/json", json);
        });
    }

    // Ping/test
    server.on("/api/info", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.onNotFound([](AsyncWebServerRequest *request){
        Log.warningln(F("404: %s"), request->url().c_str());
        request->send(404, "text/plain", "Not Found");
    });
}
