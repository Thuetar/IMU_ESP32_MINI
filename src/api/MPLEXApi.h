//MPLEXApi.h
#pragma once

#include <ESPAsyncWebServer.h>
#include "device/ads/MPLEX.h"
#include "config/ConfigManager.h"
#include "api/DeviceApi.h"
#include <ArduinoJson.h>
#include <ArduinoLog.h>

using namespace overseer::device::ads;

namespace overseer::device::api {
    class MPLEXApi : public api::DeviceApi {
    public:
        MPLEXApi(AsyncWebServer& server, MPLEX& mplex, config::ConfigManager& config);

        void begin() override;    // sets up HTTP and WebSocket routes
        void broadcast() override;     // broadcasts channel data over WebSocket at interval

    private:
        AsyncWebServer& server;
        MPLEX& mplex;
        config::ConfigManager& config;
        AsyncWebSocket ws;

        unsigned long lastBroadcast = 0;
        const unsigned long broadcastInterval = 100; // ms

        void setupRoutes();
        
        // HTTP GET handlers
        void handleChannelsJson(AsyncWebServerRequest *request);
        void handleChannelJson(AsyncWebServerRequest *request);
        void handleConfigGet(AsyncWebServerRequest *request);

        // HTTP POST handlers
        void handleConfigPostForm(AsyncWebServerRequest *request);
        void handleConfigPostJson(AsyncWebServerRequest *request, uint8_t *data, size_t len);
        void handleCalibrationPost(AsyncWebServerRequest *request);

        // WebSocket events
        void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                            AwsEventType type, void *arg, uint8_t *data, size_t len);

        // Helper methods
        int getChannelFromPath(const String& path);
        bool isValidChannel(int channel);
        JsonObject createChannelJson(JsonDocument& doc, int channel);
        JsonObject createAllChannelsJson(JsonDocument& doc);
    };

}