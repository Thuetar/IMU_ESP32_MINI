//IMUApi.cpp
#include "api/IMUApi.h"

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
        
        JsonObject windows = doc.createNestedObject("max_g_windows");
        JsonObject gx = windows["gx"].to<JsonObject>();
        writeOrderedGWindows(gx, data.max_g_windows_x);

        JsonObject gy = windows["gy"].to<JsonObject>();
        writeOrderedGWindows(gy, data.max_g_windows_y);

        JsonObject gz = windows["gz"].to<JsonObject>();
        writeOrderedGWindows(gz, data.max_g_windows_z);

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

void IMUApi::writeOrderedGWindows(JsonObject& target, const std::map<String, float>& source) {
    static const char* G_WINDOW_LABELS[] = {
        "1s", "5s", "10s", "15s", "30s", "45s", "60s",
        "1m", "5m", "10m", "15m", "30m"
    };

    for (const char* label : G_WINDOW_LABELS) {
        auto it = source.find(label);
        if (it != source.end()) {
            target[label] = it->second;
        }
    }
}

void IMUApi::handleIMUJson(AsyncWebServerRequest *request) {
    JsonDocument doc;
    const auto& data = imu.getData();
    
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
    
    JsonObject windows = doc["max_g_windows"].to<JsonObject>();

    JsonObject gx = windows["gx"].to<JsonObject>();
    writeOrderedGWindows(gx, data.max_g_windows_x);

    JsonObject gy = windows["gy"].to<JsonObject>();
    writeOrderedGWindows(gy, data.max_g_windows_y);

    JsonObject gz = windows["gz"].to<JsonObject>();
    writeOrderedGWindows(gz, data.max_g_windows_z);

    String payload;
    serializeJson(doc, payload);
    request->send(200, "application/json", payload);
}

void IMUApi::handleConfigGet(AsyncWebServerRequest *request) {    
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

