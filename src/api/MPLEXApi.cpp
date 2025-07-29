//MPLEXApi.cpp
#include "api/MPLEXApi.h"

using namespace config;
using namespace overseer::device::ads;

namespace overseer::device::api {

MPLEXApi::MPLEXApi(AsyncWebServer& server, MPLEX& mplex, ConfigManager& config)
    : server(server), mplex(mplex), config(config), ws("/mplex/ws") {}

void MPLEXApi::begin() {
    setupRoutes();
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
        onWebSocketEvent(server, client, type, arg, data, len);
    });
    server.addHandler(&ws);
}

void MPLEXApi::broadcast() {
    if (millis() - lastBroadcast >= broadcastInterval && ws.count() > 0) {
        lastBroadcast = millis();

        JsonDocument doc;
        createAllChannelsJson(doc);
        
        // Add MPLEX status
        JsonObject adcStatus = doc["adc_status"].to<JsonObject>();
        adcStatus["connected"] = mplex.isConnected();
        adcStatus["gain"] = mplex.getGain();
        adcStatus["data_rate"] = mplex.getDataRate();
        adcStatus["wire_clock"] = mplex.getWireClock();
        
        doc["timestamp"] = millis();

        String payload;
        serializeJson(doc, payload);
        ws.textAll(payload);
    }
}

void MPLEXApi::setupRoutes() {
    // GET /api/mplex - All channels data
    server.on("/api/mplex", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleChannelsJson(request);
    });

    // GET /api/mplex/channel/{0-3} - Individual channel data
    server.on("^\\/api\\/mplex\\/channel\\/([0-3])$", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleChannelJson(request);
    });

    // GET /api/mplex/config - Configuration data
    server.on("/api/mplex/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleConfigGet(request);
    });

    // POST /api/mplex/config - Update configuration
    server.on("/api/mplex/config", HTTP_POST, 
        [this](AsyncWebServerRequest *request) {
            handleConfigPostForm(request);
        },
        NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            handleConfigPostJson(request, data, len);
        });

    // POST /api/mplex/calibrate - Calibrate zero points
    server.on("/api/mplex/calibrate", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleCalibrationPost(request);
    });
}

void MPLEXApi::handleChannelsJson(AsyncWebServerRequest *request) {
    JsonDocument doc;
    createAllChannelsJson(doc);
    
    // Add MPLEX status
    JsonObject adcStatus = doc["adc_status"].to<JsonObject>();
    adcStatus["connected"] = mplex.isConnected();
    adcStatus["gain"] = mplex.getGain();
    adcStatus["data_rate"] = mplex.getDataRate();
    adcStatus["wire_clock"] = mplex.getWireClock();
    
    doc["timestamp"] = millis();

    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void MPLEXApi::handleChannelJson(AsyncWebServerRequest *request) {
    int channel = getChannelFromPath(request->url());
    
    if (!isValidChannel(channel)) {
        request->send(400, "application/json", "{\"error\":\"Invalid channel\"}");
        return;
    }

    if (!mplex.isChannelValid(channel)) {
        request->send(404, "application/json", "{\"error\":\"Channel not enabled\"}");
        return;
    }

    JsonDocument doc;
    createChannelJson(doc, channel);
    doc["timestamp"] = millis();

    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void MPLEXApi::handleConfigGet(AsyncWebServerRequest *request) {
    JsonDocument doc;
    
    // Global MPLEX settings
    JsonObject adcConfig = doc["adc"].to<JsonObject>();
    adcConfig["connected"] = mplex.isConnected();
    adcConfig["gain"] = mplex.getGain();
    adcConfig["data_rate"] = mplex.getDataRate();
    adcConfig["wire_clock"] = mplex.getWireClock();
    
    // Per-channel settings
    JsonArray channelsConfig = doc["channels"].to<JsonArray>();
    for (int i = 0; i < mplex.getChannelCount(); i++) {
        JsonObject channelConfig = channelsConfig.add<JsonObject>();
        channelConfig["channel"] = i;
        
        auto config = mplex.getChannelConfig(i);
        channelConfig["enabled"] = config.enabled;
        channelConfig["label"] = config.label;
        channelConfig["units"] = config.units;
        channelConfig["gain"] = config.gain;
        channelConfig["offset"] = config.offset;
        channelConfig["min_range"] = config.min_range;
        channelConfig["max_range"] = config.max_range;
    }

    String response;
    serializeJson(doc, response);
    
    request->send(200, "application/json", response);
}

void MPLEXApi::handleConfigPostForm(AsyncWebServerRequest *request) {
    // Handle form-based configuration updates
    bool updated = false;
    String response = "{\"status\":\"ok\",\"updated\":[";
    
    // Check for global ADC configuration parameters
    if (request->hasParam("gain", true)) {
        uint8_t gain = request->getParam("gain", true)->value().toInt();
        mplex.setGain(gain);
        response += "\"gain\",";
        updated = true;
    }
    
    if (request->hasParam("data_rate", true)) {
        uint8_t rate = request->getParam("data_rate", true)->value().toInt();
        mplex.setDataRate(rate);
        response += "\"data_rate\",";
        updated = true;
    }
    
    if (updated) {
        response.remove(response.length() - 1); // Remove trailing comma
    }
    response += "]}";
    
    request->send(200, "application/json", response);
}

void MPLEXApi::handleConfigPostJson(AsyncWebServerRequest *request, uint8_t *data, size_t len) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data, len);
    
    if (error) {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    bool updated = false;
    JsonDocument responseDoc;
    JsonArray updatedFields = responseDoc["updated"].to<JsonArray>();

    // Handle global ADC settings
    if (doc["adc"]["gain"].is<uint8_t>()) {
        mplex.setGain(doc["adc"]["gain"]);
        updatedFields.add("adc_gain");
        updated = true;
    }
    
    if (doc["adc"]["data_rate"].is<uint8_t>()) {
        mplex.setDataRate(doc["adc"]["data_rate"]);
        updatedFields.add("adc_data_rate");
        updated = true;
    }

    // Handle per-channel configuration
    if (doc["channels"].is<JsonArray>()) {
        JsonArray channels = doc["channels"];
        for (JsonObject channelConfig : channels) {
            int channel = channelConfig["channel"] | -1;
            
            if (isValidChannel(channel)) {
                ChannelConfig config = mplex.getChannelConfig(channel);
                bool channelUpdated = false;
                
                if (channelConfig["enabled"].is<bool>()) {
                    config.enabled = channelConfig["enabled"];
                    channelUpdated = true;
                }
                
                if (channelConfig["label"].is<const char*>()) {
                    config.label = channelConfig["label"].as<String>();
                    channelUpdated = true;
                }
                
                if (channelConfig["units"].is<const char*>()) {
                    config.units = channelConfig["units"].as<String>();
                    channelUpdated = true;
                }
                
                if (channelConfig["gain"].is<float>()) {
                    config.gain = channelConfig["gain"];
                    channelUpdated = true;
                }
                
                if (channelConfig["offset"].is<float>()) {
                    config.offset = channelConfig["offset"];
                    channelUpdated = true;
                }
                
                if (channelConfig["min_range"].is<float>()) {
                    config.min_range = channelConfig["min_range"];
                    channelUpdated = true;
                }
                
                if (channelConfig["max_range"].is<float>()) {
                    config.max_range = channelConfig["max_range"];
                    channelUpdated = true;
                }
                
                if (channelUpdated) {
                    mplex.setChannelConfig(channel, config);
                    updatedFields.add("channel_" + String(channel));
                    updated = true;
                }
            }
        }
    }

    responseDoc["status"] = updated ? "ok" : "no_changes";
    
    String response;
    serializeJson(responseDoc, response);
    
    request->send(200, "application/json", response);
}

void MPLEXApi::handleCalibrationPost(AsyncWebServerRequest *request) {
    JsonDocument responseDoc;
    JsonArray calibratedChannels = responseDoc["calibrated"].to<JsonArray>();
    
    int channelParam = -1;
    if (request->hasParam("channel")) {
        channelParam = request->getParam("channel")->value().toInt();
    }
    
    int samples = 100;
    if (request->hasParam("samples")) {
        samples = request->getParam("samples")->value().toInt();
        samples = constrain(samples, 10, 1000);
    }

    // Calibrate specific channel or all channels
    if (isValidChannel(channelParam)) {
        if (mplex.isChannelValid(channelParam)) {
            float zeroValue = mplex.calibrateChannelZero(channelParam, samples);
            JsonObject channelInfo = calibratedChannels.add<JsonObject>();
            channelInfo["channel"] = channelParam;
            channelInfo["zero_value"] = zeroValue;
        }
    } else {
        // Calibrate all enabled channels
        for (int i = 0; i < mplex.getChannelCount(); i++) {
            if (mplex.isChannelValid(i)) {
                float zeroValue = mplex.calibrateChannelZero(i, samples);
                JsonObject channelInfo = calibratedChannels.add<JsonObject>();
                channelInfo["channel"] = i;
                channelInfo["zero_value"] = zeroValue;
            }
        }
    }

    responseDoc["status"] = "ok";
    responseDoc["samples_used"] = samples;
    
    String response;
    serializeJson(responseDoc, response);
    
    request->send(200, "application/json", response);
}

void MPLEXApi::onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                              AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Log.notice("MPLEX WebSocket client #%u connected from %s\n", 
                      client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Log.notice("MPLEX WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            // Handle incoming WebSocket data if needed
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

// Helper methods

int MPLEXApi::getChannelFromPath(const String& path) {
    int lastSlash = path.lastIndexOf('/');
    if (lastSlash >= 0 && lastSlash < path.length() - 1) {
        return path.substring(lastSlash + 1).toInt();
    }
    return -1;
}

bool MPLEXApi::isValidChannel(int channel) {
    return mplex.isValidChannel(channel);
}

JsonObject MPLEXApi::createChannelJson(JsonDocument& doc, int channel) {
    if (!isValidChannel(channel)) {
        return JsonObject();
    }

    auto channelData = mplex.getChannelData(channel);
    auto channelConfig = mplex.getChannelConfig(channel);
    
    JsonObject channelObj = doc["channel"].to<JsonObject>();
    
    // Channel data
    channelObj["raw_value"] = channelData.raw_value;
    channelObj["voltage"] = channelData.voltage;
    channelObj["scaled_value"] = channelData.scaled_value;
    channelObj["valid"] = channelData.valid;
    channelObj["last_update"] = static_cast<uint32_t>(channelData.last_update);
    
    // Channel configuration
    channelObj["enabled"] = channelConfig.enabled;
    channelObj["label"] = channelConfig.label;
    channelObj["units"] = channelConfig.units;
    channelObj["gain"] = channelConfig.gain;
    channelObj["offset"] = channelConfig.offset;
    channelObj["min_range"] = channelConfig.min_range;
    channelObj["max_range"] = channelConfig.max_range;
    
    return channelObj;
}

JsonObject MPLEXApi::createAllChannelsJson(JsonDocument& doc) {
    JsonObject channels = doc["channels"].to<JsonObject>();
    
    for (int i = 0; i < mplex.getChannelCount(); i++) {
        auto channelData = mplex.getChannelData(i);
        auto channelConfig = mplex.getChannelConfig(i);
        
        JsonObject channelObj = channels[String(i)].to<JsonObject>();
        
        // Channel data
        channelObj["raw_value"] = channelData.raw_value;
        channelObj["voltage"] = channelData.voltage;
        channelObj["scaled_value"] = channelData.scaled_value;
        channelObj["valid"] = channelData.valid;
        channelObj["last_update"] = static_cast<uint32_t>(channelData.last_update);
        
        // Channel configuration
        channelObj["enabled"] = channelConfig.enabled;
        channelObj["label"] = channelConfig.label;
        channelObj["units"] = channelConfig.units;
        channelObj["gain"] = channelConfig.gain;
        channelObj["offset"] = channelConfig.offset;
        channelObj["min_range"] = channelConfig.min_range;
        channelObj["max_range"] = channelConfig.max_range;
    }
    
    return channels;
}

} // namespace overseer::device::api