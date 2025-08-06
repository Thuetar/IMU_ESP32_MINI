#ifndef BUILD_CONFIG_H
    #define BUILD_CONFIG_H
    /*
                    IMU_ESP32_MINI 
                    -- BUILD CONFIG -- 
                - CRITICAL BUILD VALUES - 
            i.e. Variables not part of the firmware
                217-123-535
    */

    #define FORMAT_SPIFFS_IF_FAILED true
        /**
         * GPIO -> FreeNova Line 
         * 
         * ESP PIN  FreeNova    Wire Color
         * GPIO8 = L12          Yellow 
         * GPIO9 = L15          Blue
         * 
         * GPI10 = L16
         * 11   =   17
         * 12   =   18
         * 13   =   19         
         **/ 
    // I2C INIT
    #define OVERSEER_SDA_PIN 8 //GPIO8 = L12 
    #define OVERSEER_SCL_PIN 9 //GPIO9 = L15 

    // I2C Addresses -- 0x3C Display

    //INDICATORS 
    #define STATUS_LED_GREEN 10 //FIXME: SPI flash / strapping conflicts
    #define STATUS_LED_YELLOW 11 //FIXME: SPI flash / strapping conflicts
    #define STATUS_LED_RED 12
    #define STATUS_LED_INDICATOR 13

    /** 
     * FIXME: make indexing 0 based.
    **/
    //ADS CONFIG -- ANALOG DEVICES 
    #define WCS_SENSOR_1_ADS_CHANNEL A0
    #define WCS_SENSOR_1_GAIN 0

    // Optional Devices -- ADC 0001
    //#define DC_SENSOR_1             A0
    //#define AC_SENSOR_1             A1


    //DHT11 CONFIG 
    #define DHT11_SENSOR_0__PIN 18
    #define DHT11_SENSOR_0__TYPE DHT11
#endif






