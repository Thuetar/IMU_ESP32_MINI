#pragma once
#include <esp_timer.h>

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() /* esp_timer is already initialized */
#define portGET_RUN_TIME_COUNTER_VALUE() esp_timer_get_time()

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <stdio.h>
#include <string.h>

// Provide a simple stats formatter
void vTaskGetRunTimeStats(char* buffer) {
    TaskStatus_t* taskStatusArray;
    volatile UBaseType_t arraySize, i;
    uint32_t totalRunTime;

    arraySize = uxTaskGetNumberOfTasks();
    taskStatusArray = (TaskStatus_t*)pvPortMalloc(arraySize * sizeof(TaskStatus_t));

    if (taskStatusArray != NULL) {
        arraySize = uxTaskGetSystemState(taskStatusArray, arraySize, &totalRunTime);
        if (totalRunTime > 0) {
            for (i = 0; i < arraySize; i++) {
                char line[128];
                uint32_t taskTime = taskStatusArray[i].ulRunTimeCounter;
                float percent = 100.0f * taskTime / totalRunTime;
                snprintf(line, sizeof(line), "%-16s %10lu %5.2f%%\n",
                         taskStatusArray[i].pcTaskName,
                         (unsigned long)taskTime,
                         percent);
                strcat(buffer, line);
            }
        } else {
            strcpy(buffer, "Total runtime is zero.\n");
        }
        vPortFree(taskStatusArray);
    } else {
        strcpy(buffer, "Memory allocation failed.\n");
    }
}
}
