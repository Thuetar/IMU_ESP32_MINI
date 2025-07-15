// src/devices/WifiManager.cpp
#include "WifiManager.h"

using namespace config;
WiFiManager::WiFiManager(ConfigManager &config)
    : _config(config) {}

void WiFiManager::setRetries(uint8_t count) {
    _retryCount = count;
}

void WiFiManager::setTimeout(uint32_t ms) {
    _timeoutMs = ms;
}

void WiFiManager::begin() {
    String ssid = _config.get("wifi", "ssid");
    String pass = _config.get("wifi", "password");

    if (ssid.isEmpty()) {
        Log.error("WiFi SSID not set in config!" CR);
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Log.notice("Connecting to WiFi SSID: %s" CR, ssid.c_str());

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < _timeoutMs) {
        Log.trace(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        _connected = true;
        Log.notice("WiFi connected, IP: %s" CR, WiFi.localIP().toString().c_str());
    } else {
        Log.warning("WiFi connection failed after %lu ms" CR, millis() - startAttempt);
    }
}

void WiFiManager::loop() {
    if (!_connected && WiFi.status() != WL_CONNECTED) {
        Log.warning("WiFi disconnected, attempting reconnect..." CR);
        connect();
    }
}

void WiFiManager::connect() {
    for (uint8_t attempt = 0; attempt < _retryCount; ++attempt) {
        Log.notice("WiFi reconnect attempt %d/%d" CR, attempt + 1, _retryCount);
        WiFi.reconnect();

        delay(1000);
        if (WiFi.status() == WL_CONNECTED) {
            _connected = true;
            Log.notice("WiFi reconnected, IP: %s" CR, WiFi.localIP().toString().c_str());
            return;
        }
    }

    Log.error("Failed to reconnect after %d attempts" CR, _retryCount);
    _connected = false;
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::localIP() const {
    return WiFi.localIP().toString();
}

String WiFiManager::ssid() const {
    return WiFi.SSID();
}

