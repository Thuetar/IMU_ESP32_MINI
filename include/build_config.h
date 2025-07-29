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
//INDICATORS 
#define STATUS_LED_GREEN 10 //FIXME: SPI flash / strapping conflicts
#define STATUS_LED_YELLOW 11 //FIXME: SPI flash / strapping conflicts
#define STATUS_LED_RED 12
#define STATUS_LED_INDICATOR 13

/** 
 * FIXME: make indexing 0 based.
**/
//ADS CONFIG -- ANALOG DEVICES 
#define WCS_SENSOR_1_ADS_CHANNEL 0
#define WCS_SENSOR_1_GAIN 0

//DHT11 CONFIG 
#define DHT11_SENSOR_0__PIN 18
#define DHT11_SENSOR_0__TYPE DHT11
#endif