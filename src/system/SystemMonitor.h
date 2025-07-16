//SystemMonitor.h
#pragma once

#include <Arduino.h>

namespace config {
    class ConfigManager;  // <-- forward declare here
}

namespace system_utils {

    class SystemMonitor {        
        private:
            //config::ConfigManager& config; 
            unsigned long lastLogTime = 0;
            unsigned long logInterval = 10000; // default fallback
        protected:
            config::ConfigManager& config;            
        public:
            explicit SystemMonitor(config::ConfigManager& cfg) : config(cfg) {}
            virtual ~SystemMonitor() = default;

            virtual void begin() = 0;
            virtual void update() = 0;

            virtual size_t getFreeHeap() = 0;
            virtual size_t getStackHighWaterMark() = 0;
    };

    SystemMonitor* createSystemMonitor(config::ConfigManager& config);
}


