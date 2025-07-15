#include "config/ConfigManager.h"
namespace config {
ConfigManager::ConfigManager(fs::FS &fs, const String &filePath)
    : _fs(fs), _filePath(filePath) {
    _ini.SetUnicode();
}

bool ConfigManager::begin() {
    return load();
}

const char* ConfigManager::getString(const char* section, const char* key, const char* defaultValue) const {
    const char* val = _ini.GetValue(section, key, defaultValue);
    return val ? val : defaultValue;
}

int ConfigManager::getInt(const char* section, const char* key, int defaultValue) const {
    const char* val = _ini.GetValue(section, key, nullptr);
    return val ? atoi(val) : defaultValue;
}

bool ConfigManager::getBool(const char* section, const char* key, bool defaultValue) const {
    const char* val = _ini.GetValue(section, key, nullptr);
    if (!val) return defaultValue;

    if (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0 || strcasecmp(val, "yes") == 0)
        return true;
    if (strcasecmp(val, "false") == 0 || strcmp(val, "0") == 0 || strcasecmp(val, "no") == 0)
        return false;

    return defaultValue;  // fallback on unrecognized string
}

bool ConfigManager::load() {
    Log.notice("Loading config from %s" CR, _filePath.c_str());

    File file = _fs.open(_filePath.c_str(), "r");
    if (!file) {
        Log.warning("Failed to open config file, will create new" CR);
        _dirty = true;
        return false;
    }

    size_t size = file.size();
    if (size == 0) {
        Log.warning("Config file is empty" CR);
        file.close();
        _dirty = true;
        return false;
    }

    std::unique_ptr<char[]> buffer(new char[size + 1]);
    file.readBytes(buffer.get(), size);
    buffer[size] = '\0';
    file.close();

    SI_Error err = _ini.LoadData(buffer.get());
    if (err < 0) {
        Log.error("Failed to parse config file (error %d)" CR, err);
        _dirty = true;
        return false;
    }

    _version = get("meta", "version", _version);
    Log.notice("Loaded config version %s" CR, _version.c_str());
    return true;
}

bool ConfigManager::save() {
    if (!_dirty) {
        Log.trace("Config not dirty, skipping save" CR);
        return true;
    }

    set("meta", "version", _version);
    return writeToDisk();
}

bool ConfigManager::writeToDisk() {
    Log.notice("Saving config to %s" CR, _filePath.c_str());

    std::string data;
    _ini.Save(data);

    File file = _fs.open(_filePath.c_str(), "w");
    if (!file) {
        Log.error("Failed to open config file for writing" CR);
        return false;
    }

    file.print(data.c_str());
    file.close();

    Log.notice("Config saved successfully" CR);
    _dirty = false;
    return true;
}

void ConfigManager::set(const String &section, const String &key, const String &value) {
    if (_ini.SetValue(section.c_str(), key.c_str(), value.c_str()) >= 0) {
        _dirty = true;
    }
}

String ConfigManager::get(const String &section, const String &key, const String &defaultValue) const {
    const char *value = _ini.GetValue(section.c_str(), key.c_str(), defaultValue.c_str());
    return String(value);
}

void ConfigManager::setVersion(const String &version) {
    _version = version;
    set("meta", "version", version);
}

String ConfigManager::getVersion() const {
    return _version;
}
int8_t ConfigManager::getLogLevel() const {
    return _logLevel;
}

bool ConfigManager::setBool(const char* section, const char* key, bool value) {
    _ini.SetBoolValue(section, key, value);
    return save();
}

bool ConfigManager::setFloat(const char* section, const char* key, float value) {
    _ini.SetValue(section, key, String(value, 6).c_str());
    return save();
}

float ConfigManager::getFloat(const char* section, const char* key, float defaultValue) const {
    const char* val = _ini.GetValue(section, key);
    return val ? atof(val) : defaultValue;
}
}