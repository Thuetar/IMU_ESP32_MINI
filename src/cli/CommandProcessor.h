#pragma once
//#include "devices/core/i2c_core.h"
//#include <device/core/i2c_core.h>
#include <Arduino.h>
#include <functional>
#include <map>
//#include <Wire.h>

//using namespace overseer::device::core;

class CommandProcessor {

    std::map<String, std::function<String()>> getters;
    std::map<String, std::function<void(String)>> setters;

public: 
    /*
    CommandProcessor(Device& d) : device(d) {
        getters["temp"] = [&]() { return String(device.getTemp()); };
        getters["mode"] = [&]() { return device.getMode(); };

        setters["temp"] = [&](String v) { device.setTemp(v.toInt()); };
        setters["mode"] = [&](String v) { device.setMode(v); };
    }
    */
   
    void processLine(String line) {
        line.trim();
        int space1 = line.indexOf(' ');
        int space2 = line.indexOf(' ', space1 + 1);

        String cmd = line.substring(0, space1);
        String key = line.substring(space1 + 1, space2 == -1 ? line.length() : space2);
        String val = space2 == -1 ? "" : line.substring(space2 + 1);

        if (cmd == "get") {
            if (getters.count(key)) {
                //Serial.println("key: " + key + " set to " + val);
                Serial.println(key + " = " + getters[key]());
            } else {
                Serial.println("(get) Unknown key: " + key);
            }
        } else if (cmd == "set") {
            if (setters.count(key)) {
                setters[key](val);
                Serial.println("OK: " + key + " set to " + val);
            } else {
                Serial.println("(set) Unknown key: " + key);
            }
            if (key == "log") {
                Serial.println("Setting log level not implemenented.");
            }
        } 
        else if (cmd == "clear") { //clear running data
            if (setters.count(key)) {
                setters[key](val);
                Serial.println("key: " + key + " val: " + val);
            } 
            else if (key == "data") {
                Serial.println("clear IMU Data");
                //mpu6000::data::MPUData::_max_gx = 0.0;
                //mpu6000::data::MPUData::
                /* if (val == "scan") {
                    Serial.println("Starting i2c scan");
                    I2CCore i2c;
                    i2c.begin();
                    i2c.address_scan();
                    Serial.println("debug i2c scan ENDED");
                }
                else {
                    Serial.println("Unknown i2c command");
                } */
            }
            else {
                Serial.println("(clear) Unknown Command key: " + key);
            }
        }
        else if (cmd == "debug") {
            if (setters.count(key)) {
                setters[key](val);
                Serial.println("key: " + key + " val: " + val);
            } 
            else if (key == "i2c") {
                if (val == "scan") {
                    Serial.println("Starting i2c scan");
                    //I2CCore i2c;
                    //i2c.begin();
                    //i2c.address_scan();
                    
                    Serial.println("debug i2c scan ENDED");
                }
                else {
                    Serial.println("Unknown i2c command");
                }
            }
            else {
                Serial.println("(debug) Unknown key: " + key);
            }
        } 
        
        else {
            Serial.println("Unknown command: " + cmd);
        }
    }
};
