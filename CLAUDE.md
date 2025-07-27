# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

This is an ESP32-based IMU (Inertial Measurement Unit) project using PlatformIO for remote control applications. It features WiFi connectivity, web-based APIs, real-time data streaming, and system monitoring with minimal power requirements.

## Build Commands

### Standard Build & Upload
```bash
pio run -t upload && pio device monitor
```

### SPIFFS Filesystem Operations
```bash
# Update device SPIFFS/Flash filesystem
pio run --target buildfs && pio run --target uploadfs
```

### Deep Clean (when dependencies are corrupted)
```bash
pio run --target clean
rm -rf .pio/libdeps
rm -rf .pio/build
rm -rf .pio/.cache
rm -rf ~/.platformio/lib
```

### Monitoring
```bash
pio device monitor
```

## Architecture

### Core Components

- **Main Controller** (`src/main.cpp`): Orchestrates system initialization, manages the main event loop with IMU data processing, serial command handling, and system monitoring
- **Configuration System** (`config/ConfigManager`): Centralized configuration management using SPIFFS for persistent storage
- **IMU System** (`device/IMU/MPU6000/`): MPU6000 sensor integration with filtering, smoothing algorithms, and data collection
- **Web Server** (`src/web/WebServerManager`): AsyncWebServer-based HTTP/WebSocket server for real-time data streaming
- **WiFi Management** (`src/net/WifiManager`): WiFi connection handling with retry logic and connection monitoring
- **System Monitor** (`src/system/SystemMonitor`): ESP32-specific system monitoring (heap, stack, CPU usage, uptime)
- **CLI Interface** (`src/cli/CommandProcessor`): Serial command processor for debugging and configuration

### Data Flow
1. IMU data is collected and processed in the main loop with configurable filtering
2. Data is exposed via REST API endpoints (`/api/imu`, `/api/system`) and WebSocket streams
3. Web interface provides real-time visualization served from SPIFFS
4. System monitoring data is collected and exposed via `/dx` diagnostics endpoint

## Development Configuration

### Hardware Setup
- **Board**: NodeMCU-32S (ESP32)
- **Upload Port**: `/dev/cu.usbserial-0001`
- **Upload Speed**: 230400 baud
- **Monitor Speed**: 115200 baud
- **Filesystem**: SPIFFS

### Key Libraries
- **ArduinoJson 7.4.2**: JSON handling for APIs
- **ArduinoLog 1.1.1**: Logging system
- **I2Cdevlib**: MPU6050/6000 sensor communication
- **AsyncTCP + ESPAsyncWebServer**: Web server framework
- **libOverseer**: Custom shared library (symlinked)

### Build Flags
- **C++ Standard**: gnu++17
- **System Monitoring**: Enabled with FreeRTOS stats
- **Debug Features**: Exception decoder enabled, watchdog disabled for testing
- **Panic Handling**: Configured to halt on panic for debugging

## API Endpoints

### IMU Data
- **GET** `/api/imu` - Current IMU readings and statistics
- **WebSocket** - Real-time IMU data streaming

### System Monitoring
- **GET** `/api/system` - Basic system metrics (heap, stack watermark)
- **GET** `/dx` - Comprehensive diagnostics (system + IMU statistics with windowed max values)

### Configuration
- **GET/POST** `/api/config` - Configuration management

## Serial Commands (Debug CLI)

Available via serial monitor at 115200 baud:

- `debug i2c scan` - Scan I2C bus for connected devices
- `get <key>` - Get configuration value
- `set <key> <value>` - Set configuration value
- `clear data` - Reset IMU data counters

## Important Notes

### libOverseer Dependency
The project uses a custom shared library (`libOverseer`) via symlink. Ensure this library is available at the specified path for successful compilation.

### FreeRTOS Configuration
Special requirement: `INCLUDE_xTaskGetIdleTaskHandle` must be set to 1 in FreeRTOSConfig.h for CPU usage monitoring to function properly.

### SPIFFS Filesystem
Web interface files are served from SPIFFS. Use `buildfs` and `uploadfs` targets to update the filesystem after modifying web assets.

### Debug Configuration
- Exception decoder is enabled for crash analysis (remove for production)
- Watchdog is disabled to prevent auto-reboot during testing
- Verbose logging is available and configurable via serial commands