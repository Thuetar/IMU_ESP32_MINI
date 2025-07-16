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
//extern "C" unsigned long ulTaskGetRunTimeCounter(TaskHandle_t xTask);
//extern "C" unsigned long uxTaskGetRunTimeCounter(TaskHandle_t xTask);
using namespace config;
using namespace system_utils;

// Declare external idle task handle
//extern TaskHandle_t xIdleTaskHandle;

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

        // Capture initial CPU timing values
        lastCPUTime = esp_timer_get_time();  // microseconds
        //lastIdleTicks = uxTaskGetRunTimeCounter(xIdleTaskHandle);
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
            Log.infoln(F("[System] CPU usage: %.2f%%"), getCPUUsagePercent());
        }
    }

    size_t getFreeHeap() override {
        return esp_get_free_heap_size();
    }

    size_t getStackHighWaterMark() override {
        return uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t);
    }

    float getCPUUsagePercent()  {
        static char buffer[512];
        vTaskGetRunTimeStats(buffer);
        Log.infoln("[System] Task stats:\n%s", buffer);

        // Optionally, parse and extract IDLE % and subtract from 100
        // but this log alone is very useful.
        return 0.0f;  // Or parse and return actual percentage
    }

    /*
    float getCPUUsagePercent() {
        int64_t now = esp_timer_get_time();  // µs since boot
        uint32_t idleTicks = uxTaskGetRunTimeCounter(xIdleTaskHandle);

        int64_t elapsedTime = now - lastCPUTime;
        uint32_t elapsedIdle = idleTicks - lastIdleTicks;

        lastCPUTime = now;
        lastIdleTicks = idleTicks;

        // Approximate CPU usage (1 - idle time / elapsed time)
        if (elapsedTime <= 0) return 0.0f;

        float usage = 100.0f * (1.0f - (float)elapsedIdle / (float)elapsedTime);
        if (usage < 0.0f) usage = 0.0f;
        if (usage > 100.0f) usage = 100.0f;

        return usage;
    }

    float getCPUUsagePercent() {
        int64_t now = esp_timer_get_time();
        unsigned long idleRuntime = uxTaskGetRunTimeCounter(xIdleTaskHandle);

        int64_t elapsedTime = now - lastCPUTime;
        unsigned long elapsedIdle = idleRuntime - lastIdleTicks;

        lastCPUTime = now;
        lastIdleTicks = idleRuntime;

        if (elapsedTime <= 0) return 0.0f;

        float usage = 100.0f * (1.0f - (float)elapsedIdle / (float)elapsedTime);
        if (usage < 0.0f) usage = 0.0f;
        if (usage > 100.0f) usage = 100.0f;

        return usage;
    }
*/

private:
    unsigned long lastUpdate = 0;
    unsigned long monitorInterval = 10000;
    bool logHeap = true;
    bool logStack = true;
    bool logCpu = true;

    int64_t lastCPUTime = 0;
    uint32_t lastIdleTicks = 0;
};

// factory
SystemMonitor* createSystemMonitor(config::ConfigManager& config) {
    static ESP32SystemMonitor monitor(config);
    return &monitor;
}

}  // namespace system_utils




