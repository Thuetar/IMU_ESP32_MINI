//IMUApi.cpp
#include "api/IMUApi.h"
#include <ArduinoJson.h>
#include <ArduinoLog.h>
using namespace config;

namespace mpu6000 {

IMUApi::IMUApi(AsyncWebServer& server, MPU6000& imu, ConfigManager& config)
    : server(server), imu(imu), config(config), ws("/imu/ws") {}

void IMUApi::begin() {
    setupRoutes();
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
        onWebSocketEvent(server, client, type, arg, data, len);
    });
    server.addHandler(&ws);
}

void IMUApi::broadcast() {
    if (millis() - lastBroadcast >= broadcastInterval && ws.count() > 0) {
        lastBroadcast = millis();

        const auto& data = imu.getData();
        JsonDocument doc;
        doc["pitch"] = data.pitch_deg;
        doc["roll"] = data.roll_deg;

        JsonArray smooth = doc["smooth_g"].to<JsonArray>();
        smooth.add(data.gx_smooth);
        smooth.add(data.gy_smooth);
        smooth.add(data.gz_smooth);

        JsonArray max_g = doc["max_g_lifetime"].to<JsonArray>();
        max_g.add(data.max_gx);
        max_g.add(data.max_gy);
        max_g.add(data.max_gz);
        /*
        JsonObject windows = doc.createNestedObject("max_g_windows");

        JsonObject gx = windows.createNestedObject("gx");
        for (const auto& [label, value] : data.max_g_windows_x) gx[label] = value;

        JsonObject gy = windows.createNestedObject("gy");
        for (const auto& [label, value] : data.max_g_windows_y) gy[label] = value;

        JsonObject gz = windows.createNestedObject("gz");
        for (const auto& [label, value] : data.max_g_windows_z) gz[label] = value;
        */
        String payload;
        serializeJson(doc, payload);
        ws.textAll(payload);
    }
}

void IMUApi::setupRoutes() {
    server.on("/imu", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleIMUJson(request);
    });

    server.on("/imu/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleConfigGet(request);
    });

    server.on("/imu/config", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleConfigPostForm(request);
    });

    server.on("/imu/config", HTTP_POST, nullptr, nullptr,
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len,
               size_t index, size_t total) {
            handleConfigPostJson(request, data, len);
    });
}

void IMUApi::handleIMUJson(AsyncWebServerRequest *request) {
    const auto& data = imu.getData();
    //DynamicJsonDocument doc(512);
    JsonDocument doc;
    
    doc["pitch"] = data.pitch_deg;
    doc["roll"] = data.roll_deg;

    JsonArray smooth = doc["smooth_g"].to<JsonArray>();
    smooth.add(data.gx_smooth);
    smooth.add(data.gy_smooth);
    smooth.add(data.gz_smooth);

    JsonArray max_g = doc["max_g_lifetime"].to<JsonArray>();
    max_g.add(data.max_gx);
    max_g.add(data.max_gy);
    max_g.add(data.max_gz);
    //*
    //JsonObject windows = doc.createNestedObject("max_g_windows");
    JsonObject windows = doc["max_g_windows"].to<JsonObject>();

    //JsonObject gx = windows.createNestedObject("gx");
    JsonObject gx = windows["gx"].to<JsonObject>();
    for (const auto& [label, value] : data.max_g_windows_x) gx[label] = value;

    //JsonObject gy = windows.createNestedObject("gy");
    JsonObject gy = windows["gy"].to<JsonObject>();
    for (const auto& [label, value] : data.max_g_windows_y) gy[label] = value;

    //JsonObject gz = windows.createNestedObject("gz");
    JsonObject gz = windows["gz"].to<JsonObject>();
    for (const auto& [label, value] : data.max_g_windows_z) gz[label] = value;
    //*/
    String payload;
    serializeJson(doc, payload);
    request->send(200, "application/json", payload);
}

void IMUApi::handleConfigGet(AsyncWebServerRequest *request) {
    //DynamicJsonDocument doc(256);
    JsonDocument doc;
    doc["smoothing_enabled"] = config.getBool("imu", "smoothing_enabled", false);
    doc["spike_threshold"] = config.getFloat("imu", "spike_threshold", 3.0f);

    String payload;
    serializeJson(doc, payload);
    request->send(200, "application/json", payload);
}

void IMUApi::handleConfigPostForm(AsyncWebServerRequest *request) {
    if (request->hasParam("smoothing_enabled", true)) {
        bool val = request->getParam("smoothing_enabled", true)->value() == "true";
        config.setBool("imu", "smoothing_enabled", val);
    }
    if (request->hasParam("spike_threshold", true)) {
        float val = request->getParam("spike_threshold", true)->value().toFloat();
        config.setFloat("imu", "spike_threshold", val);
    }
    request->send(204);
}

void IMUApi::handleConfigPostJson(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
    //DynamicJsonDocument doc(256);
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if (err) {
        request->send(400, "application/json", "{\"error\":\"invalid json\"}");
        return;
    }

    if (doc.containsKey("smoothing_enabled")) {
        config.setBool("imu", "smoothing_enabled", doc["smoothing_enabled"]);
    }
    if (doc.containsKey("spike_threshold")) {
        config.setFloat("imu", "spike_threshold", doc["spike_threshold"]);
    }

    request->send(204);
}

void IMUApi::onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                              AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Log.notice("WebSocket client connected: #%u\n", client->id());
            break;
        case WS_EVT_DISCONNECT:
            Log.notice("WebSocket client disconnected: #%u\n", client->id());
            break;
        case WS_EVT_DATA:
            // Optionally handle messages
            break;
        default:
            break;
    }
}

} // namespace mpu6000

