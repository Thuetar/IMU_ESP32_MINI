//config/ConfigManager.h
#pragma once

#include <Arduino.h>
#include <SimpleIni.h>
#include <FS.h>
#include <ArduinoLog.h>

namespace config {
    class ConfigManager {
    public:
        ConfigManager(fs::FS &fs, const String &filePath = "/config.ini");

        bool begin();
        bool load();
        bool save();

        void set(const String &section, const String &key, const String &value);
        String get(const String &section, const String &key, const String &defaultValue = "") const;
        
        const char* getString(const char* section, const char* key, const char* defaultValue = "") const;
        int getInt(const char* section, const char* key, int defaultValue = 0) const;
        bool getBool(const char* section, const char* key, bool defaultValue = false) const;
        float getFloat(const char* section, const char* key, float defaultValue) const;
        bool setFloat(const char* section, const char* key, float value);
        bool setBool(const char* section, const char* key, bool value);

        void setVersion(const String &version);
        String getVersion() const;
        int8_t getLogLevel() const;

    private:
        fs::FS &_fs;
        String _filePath;
        mutable CSimpleIniA _ini;
        bool _dirty = false;
        String _version = "1.0.0";
        int8_t _logLevel;
        bool writeToDisk();
    };
}