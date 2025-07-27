#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H
/*
                   IMU_ESP32_MINI 
                -- BUILD CONFIG -- 
            - CRITICAL BUILD VALUES - 
        i.e. Variables not part of the firmware
    
*/

#define FORMAT_SPIFFS_IF_FAILED true

#define OVERSEER_SDA_PIN 8
#define OVERSEER_SCL_PIN 9
#define STATUS_LED_GREEN 10
#define STATUS_LED_YELLOW 11
#define STATUS_LED_RED 12
#define STATUS_LED_INDICATOR 13

#define WCS_SENSOR_1_ADS_CHANNEL 0
#define WCS_SENSOR_1_GAIN 0
#endif