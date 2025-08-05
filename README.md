# What is it?

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

## MPLEX Api
** MPLEX Data **
{
  "channels": {
    "0": {
      "current": 1.23,
      "voltage": 2.45,
      "current_smooth": 1.21,
      "max_current": 5.67,
      "max_current_windows": {
        "1s": 2.1, "5s": 3.2, "30s": 4.5
      },
      "samples_per_second": 98.5,
      "is_calibrated": true,
      "valid_reading": true
    }
    // ... channels 1-3
  },
  "mplex_status": {
    "connected": true,
    "zero_voltage": 2.5
  },
  "timestamp": 1234567890
}

*** *Usage Pattern for Rest  API,*** 
MPLEX mplex;
std::vector<WCS1800*> energySensors = {&wcs0, &wcs1, &wcs2, &wcs3};
MPLEXApi energyApi(server, mplex, energySensors, configManager);
void setup() {
    energyApi.begin();
}
void loop() {
    energyApi.broadcast();
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

___________

Safe GPIOs for general-purpose use
These are not connected to flash, boot, or other critical functions:

GPIO	Notes
0	OK to use; also bootstrapping pin (must be LOW to enter download mode)
1	OK
2	OK
3	OK
4–5	OK
6–9	❌ Avoid — used for SPI flash
**10–11	❌ Avoid — SPI flash / strapping conflicts** <---FIX
12–13	OK
14–17	OK
18–21	OK
26–33	OK 
34–39	OK
40–46	OK (just avoid those marked reserved on schematic)
47	OK — often used for button input
48	OK — often used for LED or output
🚫 Pins to avoid or use with caution
GPIO	Reason
6–11	Used for QSPI flash — don't touch
45, 46	Input-only pins — no output possible
0	Used for boot mode — keep HIGH during boot unless flashing
Strapping Pins (0, 3, 45, etc.)	Affects boot behavior — check with caution
✅ Good choices for:
Digital IO: 1, 2, 3, 4, 12–21, 26–33
Analog input (ADC1): 1–10, 11, 12, 13 — check your specific S3 variant
I2C (recommended):
SDA = GPIO2, SCL = GPIO1
But you can assign to any GPIO using Wire.begin(sda, scl)










