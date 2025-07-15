// src/devices/WifiManager.h
#pragma once
#include <Arduino.h>
#include <ArduinoLog.h>
#include <WiFi.h>
#include "../config/ConfigManager.h"

class WiFiManager {
public:
    explicit WiFiManager(config::ConfigManager &config);

    void begin();                    // Connect using stored credentials (blocking)
    void loop();                     // Reconnect if disconnected
    bool isConnected() const;

    String localIP() const;
    String ssid() const;

    void setRetries(uint8_t count);
    void setTimeout(uint32_t ms);

private:
    config::ConfigManager &_config;
    uint8_t _retryCount = 10;
    uint32_t _timeoutMs = 10000;
    bool _connected = false;

    void connect();
};

