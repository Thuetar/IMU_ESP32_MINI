//src/system/SystemMonitor_esp32.cpp
#include "freertos_runtime_esp32.h" // configure stuff for build.. macros technically
#include "SystemMonitor.h"
#include <ArduinoLog.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include "config/ConfigManager.h"

extern "C" void vTaskGetRunTimeStats(char* buffer);

using namespace config;
using namespace system_utils;

namespace system_utils {

    class ESP32SystemMonitor : public SystemMonitor {
        public:
            ESP32SystemMonitor(config::ConfigManager& cfg) : SystemMonitor(cfg) {}

            void begin() override {
                lastUpdate = 0;
                monitorInterval = config.getInt("system", "monitor_interval", 10000);
                logHeap = config.getBool("system", "log_heap", true);
                logStack = config.getBool("system", "log_stack", true);
                logCpu = config.getBool("system", "log_cpu", true);
            }

            void update() override {
                unsigned long now = millis();
                if (now - lastUpdate < monitorInterval) return;
                lastUpdate = now;

                if (logHeap) {
                    Log.infoln(F("[System] Free heap: %u bytes"), getFreeHeap());
                }
                if (logStack) {
                    Log.infoln(F("[System] Stack high watermark: %u bytes"), getStackHighWaterMark());
                }
                if (logCpu) {
                    float cpu = getCPUUsagePercent();
                    Log.infoln(F("[System] CPU usage: %.2f%%"), cpu);
                }
            }

            size_t getFreeHeap() override {
                return esp_get_free_heap_size();
            }

            size_t getStackHighWaterMark() override {
                return uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t);
            }

            float getCPUUsagePercent() {
                static char buffer[512];
                memset(buffer, 0, sizeof(buffer));
                vTaskGetRunTimeStats(buffer);
                Log.infoln("vTask Stats:\n%s", buffer);

                // Parse for "IDLE" task line
                float idlePercent = 0.0f;
                char* line = strtok(buffer, "\r\n");
                while (line != nullptr) {
                    if (strstr(line, "IDLE") != nullptr) {
                        // Expected format: <task name>\t<runtime>\t<percentage>\r\n
                        char* percentStr = strrchr(line, '\t');
                        if (percentStr) {
                            percentStr++;  // skip the tab
                            idlePercent = atof(percentStr);
                            break;
                        }
                    }
                    line = strtok(nullptr, "\r\n");
                }

                float cpuUsage = 100.0f - idlePercent;
                if (cpuUsage < 0.0f) cpuUsage = 0.0f;
                if (cpuUsage > 100.0f) cpuUsage = 100.0f;
                return cpuUsage;
            }

        private:
            unsigned long lastUpdate = 0;
            unsigned long monitorInterval = 10000;
            bool logHeap = true;
            bool logStack = true;
            bool logCpu = true;
    };

    // factory
    SystemMonitor* createSystemMonitor(config::ConfigManager& config) {
        static ESP32SystemMonitor monitor(config);
        return &monitor;
    }

}  // namespace system_utils
