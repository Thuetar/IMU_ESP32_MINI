//IMUApi.h
#pragma once

#include <ESPAsyncWebServer.h>
#include "device/IMU/MPU6000/MPU6000.h"
#include "config/ConfigManager.h"
#include "api/DeviceApi.h"
#include <ArduinoJson.h>
#include <ArduinoLog.h>

using namespace overseer::device::imu;

namespace overseer::device::api {
    class IMUApi : public api::DeviceApi {
    public:
        IMUApi(AsyncWebServer& server, MPU6000& imu, config::ConfigManager& config);

        void begin() override;    // sets up HTTP and WebSocket routes
        void broadcast() override;     // broadcasts IMU data over WebSocket at interval

    private:
        AsyncWebServer& server;
        MPU6000& imu;
        config::ConfigManager& config;
        AsyncWebSocket ws;

        unsigned long lastBroadcast = 0;
        const unsigned long broadcastInterval = 100; // ms

        void setupRoutes();
        void writeOrderedGWindows(JsonObject& target, const std::map<String, float>& source);
        // HTTP GET handlers
        void handleIMUJson(AsyncWebServerRequest *request);
        
        void handleConfigGet(AsyncWebServerRequest *request);

        // HTTP POST handlers
        void handleConfigPostForm(AsyncWebServerRequest *request);
        void handleConfigPostJson(AsyncWebServerRequest *request, uint8_t *data, size_t len);

        // WebSocket events
        void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                            AwsEventType type, void *arg, uint8_t *data, size_t len);
    };

}