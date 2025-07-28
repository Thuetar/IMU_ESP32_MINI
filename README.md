# IMU for RC :)

## Features: 
* Minimal Power REquirements
* WiFi Transmit Readings



## Build Requirements
### Espressif API (Special) Requirements
Edge case maybe? 
Pulling the cpu and task data requires some deeeep internals where maybe coverage couldn've been better. In anycase, the macro INCLUDE_xTaskGetIdleTaskHandle is not defined... and needs to be 1 in FreeRTOSConfig.h.

## Commands
### DEBUG 
- i2c scan



## Links of goodness
https://www.espboards.dev/esp32/nodemcu-32s/

### Display? 
https://www.waveshare.com/product/displays/21.5inch-fhd-monitor.htm?sku=24094

## Examples
Example payload from /api/system:
{
  "free_heap": 143216,
  "stack_high_watermark": 3480
}

### Diagnostics API
Example payload diagnostics API payload served at /dx
{
  "system": {
    "free_heap": 187320,
    "stack_watermark": 2356,
    "cpu_usage_percent": 31.2, // optional, estimate
    "uptime_ms": 23123345
  },
  "imu": {
    "max_g": {
      "x": 1.92,
      "y": 2.10,
      "z": 1.03
    },
    "window_max_g": {
      "1s": { "x": 1.2, "y": 1.4, "z": 1.1 },
      "5s": { "x": 1.7, "y": 1.8, "z": 1.0 },
      "30s": { "x": 2.1, "y": 2.4, "z": 1.3 }
    },
    "sample_stats": {
      "total_samples": 482932,
      "dropped_samples": 4,
      "samples_per_second": 96.7
    }
  }
}

## Helpful Commands

### Builds and Whatnot
Scripts in the raw.
##### Update Device SPIFFS / Flash
pio run --target buildfs && pio run --target uploadfs

pio run -t upload && pio device monitor

#### Clean... Ulta Deep
pio run --target clean
rm -rf .pio/libdeps
rm -rf .pio/build
rm -rf .pio/.cache
rm -rf ~/.platformio/lib


