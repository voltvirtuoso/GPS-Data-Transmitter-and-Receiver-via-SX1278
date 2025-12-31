# GPS-Data-Transmitter-and-Receiver-via-SX1278

A robust, production-ready GPS tracking system using LoRa SX1278 modules for long-range wireless communication. Features dual triggering mechanisms (button + timer), interrupt-driven architecture, and comprehensive GPS data handling.

## Table of Contents
- [Features](#features)
- [System Overview](#system-overview)
- [Hardware Requirements](#hardware-requirements)
- [Pin Configuration](#pin-configuration)
- [Software Dependencies](#software-dependencies)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Advanced Features](#advanced-features)
- [Troubleshooting](#troubleshooting)
- [Customization](#customization)
- [Contributing](#contributing)
- [License](#license)

## Features

- **Dual Triggering Mechanisms**:
  - **Button Trigger**: Immediate transmission via hardware interrupt (with debouncing)
  - **Timer Trigger**: Automatic transmission every 60 seconds (configurable)
- **Professional Interrupt Handling**:
  - Non-blocking ISR with proper debouncing
  - Critical section protection using `portDISABLE_INTERRUPTS()`
  - Volatile flag management for thread safety
- **Comprehensive GPS Data**:
  - Latitude and Longitude (6 decimal precision)
  - Altitude in meters
  - UTC timestamp with date
  - Trigger type identification (BUTTON/TIMER)
- **Robust Communication**:
  - LoRa SX1278 with 433MHz frequency
  - Configurable spreading factor (SF10) for optimal range/reliability
  - High transmission power (20dBm) for extended range
- **Error Handling**:
  - GPS fix validation before transmission
  - Packet parsing with data validation
  - Graceful failure handling with detailed logging
- **Low Power Optimized**:
  - Efficient sleep cycles with `delay(10)`
  - Minimal CPU usage during idle periods
  - Watchdog timer protection

## System Overview

This system consists of two ESP32-based nodes communicating via LoRa SX1278 modules:

### Transmitter Node
- **Primary Function**: Acquire GPS data and transmit via LoRa
- **Trigger Mechanisms**:
  - **Hardware Button**: Immediate transmission when pressed (interrupt-driven)
  - **Timer**: Automatic transmission every 60 seconds
- **GPS Handling**:
  - Uses TinyGPS++ library for efficient parsing
  - 1-second acquisition window for valid GPS fixes
  - Continuous GPS data feeding for up-to-date information
- **Packet Format**:
  ```
  --- LoRa GPS Packet ---
  TRIGGER_TYPE: BUTTON/TIMER
  LAT: 37.774900
  LONG: -122.419400
  ALT: 12.5
  Time (UTC): 2025/12/31,14:30:45
  ```

### Receiver Node
- **Primary Function**: Receive and parse GPS data packets
- **Data Processing**:
  - Real-time packet monitoring
  - Structured data extraction with validation
  - Human-readable output formatting
- **Error Detection**:
  - Packet size validation
  - GPS data format verification
  - Fallback handling for corrupt packets

## Hardware Requirements

### Common Components (Both Nodes)
- **ESP32 Development Board** (ESP32 DevKit v4 recommended)
- **LoRa SX1278 Module** (433MHz version)
- **Jumper Wires** (male-to-male and male-to-female)
- **Breadboard** (for prototyping)
- **5V Power Supply** (2A minimum)

### Transmitter Node Additional Components
- **GPS Module** (NEO-6M, NEO-7M, or similar UART GPS module)
- **Push Button** (tactile switch)
- **10KΩ Resistor** (for button pull-up)

### Receiver Node Additional Components
- **USB-to-Serial Adapter** (for data logging/monitoring)
- **Optional**: OLED Display for real-time GPS visualization

## Pin Configuration

### ESP32 DevKit v4 Pinout (Both Nodes)
| Function | GPIO Pin | Description |
|----------|----------|-------------|
| **LoRa SS** | GPIO 5 | Slave Select for LoRa module |
| **LoRa RST** | GPIO 22 | Reset pin for LoRa module |
| **LoRa DIO0** | GPIO 21 | Interrupt pin for LoRa module |

### Transmitter Node Additional Pins
| Function | GPIO Pin | Description |
|----------|----------|-------------|
| **GPS TX** | GPIO 16 | GPS module TX (connects to ESP32 RX) |
| **GPS RX** | GPIO 17 | GPS module RX (connects to ESP32 TX) |
| **Button** | GPIO 4 | Push button for manual triggering (with internal pull-up) |

### Receiver Node
- Uses only the common LoRa pins listed above
- Serial output via USB for monitoring and data logging

## Software Dependencies

### Required Libraries
1. **LoRa Library** by Sandeep Mistry
   - [GitHub Repository](https://github.com/sandeepmistry/arduino-LoRa)
2. **TinyGPS++ Library** by Mikal Hart
   - [GitHub Repository](https://github.com/mikalhart/TinyGPSPlus)
3. **ESP32 Board Support** (Arduino Core)
   - Version 2.0.0 or later

### Library Installation (Arduino IDE)
1. Open Arduino IDE
2. Go to **Sketch** > **Include Library** > **Manage Libraries**
3. Search and install:
   - `LoRa`
   - `TinyGPSPlus`
4. Go to **File** > **Preferences**
5. Add ESP32 board manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
6. Go to **Tools** > **Board** > **Boards Manager**
7. Search for "esp32" and install the latest version

## Installation

### Repository Setup
```bash
git clone https://github.com/voltvirtuoso/GPS-Data-Transmitter-and-Receiver-via-SX1278.git
cd GPS-Data-Transmitter-and-Receiver-via-SX1278
```

### Arduino IDE Setup
1. Open Arduino IDE
2. Go to **File** > **Open** and select:
   - `GPS_Transmitter/GPS_Transmitter.ino` for transmitter
   - `GPS_Receiver/GPS_Receiver.ino` for receiver
3. Select board: **Tools** > **Board** > **ESP32 Arduino** > **ESP32 Dev Module**
4. Select port: **Tools** > **Port** > (your ESP32 port)
5. Upload the sketch

### Serial Monitor Setup
- **Baud Rate**: 115200
- **Line Ending**: Both NL & CR
- **Monitor Both Nodes**: Use separate serial monitor windows for transmitter and receiver

## Configuration

### Key Configuration Parameters

#### In `GPS_Transmitter.ino`:
```cpp
// LoRa Configuration
#define LORA_SS 5
#define LORA_RST 22
#define LORA_DIO0 21
const long frequency = 433E6; // 433MHz frequency

// GPS Configuration
#define RX 16        // GPS TX to ESP32 RX
#define TX 17        // GPS RX to ESP32 TX
#define GPS_BAUD 9600

// Button Configuration
#define BUTTON_PIN 4
#define ISR_DEBOUNCE_DELAY 200 // 200ms debounce delay

// Transmission Intervals
const unsigned long TRANSMIT_INTERVAL = 60000; // 60 seconds
```

#### In `GPS_Receiver.ino`:
```cpp
// LoRa Configuration (MUST match transmitter)
#define LORA_SS 5
#define LORA_RST 22
#define LORA_DIO0 21
const long frequency = 433E6; // 433MHz frequency
```

### Calibration Guide
1. **GPS Module Calibration**:
   - Place GPS module in open sky area for first-time calibration
   - Allow 5-10 minutes for satellite acquisition
   - Check serial output for valid GPS fixes

2. **LoRa Range Testing**:
   - Start with maximum power: `LoRa.setTxPower(20)` for testing
   - Reduce power gradually for power-sensitive applications
   - Test different spreading factors (SF7-SF12) for optimal range

3. **Button Debouncing**:
   - Adjust `ISR_DEBOUNCE_DELAY` based on button quality:
     - Cheap buttons: 200-300ms
     - Quality buttons: 50-100ms
     - Tactile switches: 10-50ms

## Usage

### Transmitter Node Operation
1. **Power On Sequence**:
   - GPS module acquires satellite fix (1-5 minutes first time)
   - LoRa module initializes and connects
   - Serial monitor shows system status

2. **Triggering Transmissions**:
   - **Automatic**: Every 60 seconds (configurable)
   - **Manual**: Press button for immediate transmission
   - **Serial Output**:
     ```
     [INTERRUPT] Triggering immediate LoRa transmission.
     --- LoRa GPS Packet ---
     TRIGGER_TYPE: BUTTON
     LAT: 37.774900
     LONG: -122.419400
     ALT: 12.5
     Time (UTC): 2025/12/31,14:30:45
     ----------------------------
     Packet Transmitted via LoRa
     ```

### Receiver Node Operation
1. **Monitoring Mode**:
   - Continuously listens for LoRa packets
   - Parses and displays GPS data in real-time
   - **Serial Output**:
     ```
     [RECEIVED] Packet size: 152 bytes
     ----------------------------
     --- LoRa GPS Packet ---
     TRIGGER_TYPE: BUTTON
     LAT: 37.774900
     LONG: -122.419400
     ALT: 12.5
     Time (UTC): 2025/12/31,14:30:45
     ----------------------------
     [GPS DATA PARSED]
     Trigger Type: BUTTON
     Latitude: 37.774900
     Longitude: -122.419400
     Altitude: 12.5 meters
     UTC Time: 2025/12/31,14:30:45
     ----------------------------
     ```

### Practical Applications
- **Asset Tracking**: Attach transmitter to vehicles, equipment, or pets
- **Field Research**: Monitor wildlife movements or environmental sensors
- **Emergency Systems**: SOS location broadcasting with button trigger
- **Drone Navigation**: Waypoint logging and telemetry backhaul
- **Marine Applications**: Boat tracking with water-resistant enclosures

## Advanced Features

### Interrupt-Driven Architecture
The transmitter uses a professional interrupt handling system:

```cpp
void IRAM_ATTR buttonISR() {
    unsigned long currentInterruptTime = millis();
    if (currentInterruptTime - lastButtonInterruptTime > ISR_DEBOUNCE_DELAY) {
        if (digitalRead(BUTTON_PIN) == LOW) {
            buttonPressed = true;
            lastButtonInterruptTime = currentInterruptTime;
        }
    }
}

// Main loop handles the actual transmission
if (buttonPressed) {
    portDISABLE_INTERRUPTS();
    buttonPressed = false;
    portENABLE_INTERRUPTS();
    triggerLoRaSend(true); // true = button triggered
}
```

### Data Packet Structure
The system uses a human-readable packet format that's easy to parse and debug:

```
--- LoRa GPS Packet ---
TRIGGER_TYPE: BUTTON
LAT: 37.774900
LONG: -122.419400
ALT: 12.5
Time (UTC): 2025/12/31,14:30:45
```

### Power Management
For battery-powered applications, add these optimizations:

```cpp
// Deep sleep mode between transmissions (transmitter)
void enterDeepSleep() {
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, LOW); // Button wake-up
    esp_sleep_enable_timer_wakeup(TRANSMIT_INTERVAL * 1000);
    Serial.println("Entering deep sleep...");
    esp_deep_sleep_start();
}

// Wake-up handling
void setup() {
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("[WAKEUP] Timer triggered");
        // Normal timer-based transmission
    } else if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("[WAKEUP] Button triggered");
        // Button-triggered transmission
    }
    // Rest of setup...
}
```

## Troubleshooting

### Common Issues and Solutions

| Issue | Symptoms | Solution |
|-------|----------|----------|
| **No GPS Fix** | "No valid GPS fix found" warnings | 1. Ensure GPS antenna has clear sky view<br>2. Check GPS baud rate matches module<br>3. Verify GPS TX/RX connections<br>4. Allow 10+ minutes for first fix |
| **LoRa Communication Failure** | No packets received | 1. Verify pin connections match configuration<br>2. Check frequency matches on both nodes<br>3. Ensure antennas are properly connected<br>4. Test with maximum power temporarily |
| **Button Not Responding** | No interrupt triggering | 1. Check button wiring (pull-up configuration)<br>2. Increase debounce delay<br>3. Test button continuity with multimeter<br>4. Try different GPIO pin |
| **Packet Corruption** | Garbled GPS data | 1. Reduce transmission power<br>2. Decrease spreading factor<br>3. Add packet validation checksum<br>4. Reduce transmission distance |
| **System Lockups** | Nodes stop responding | 1. Add watchdog timer<br>2. Reduce GPS parsing time<br>3. Check power supply stability<br>4. Add memory management |

### Debugging Commands
Add these debug functions to your sketches:

```cpp
// GPS Debug Function (Transmitter)
void debugGPSStatus() {
    Serial.print("[GPS DEBUG] Satellites: ");
    Serial.print(gps.satellites.value());
    Serial.print(", HDOP: ");
    Serial.print(gps.hdop.value());
    Serial.print(", Fix Quality: ");
    Serial.println(gps.satellites.value() > 0 ? "Valid" : "No Fix");
}

// LoRa Signal Debug (Receiver)
void debugLoRaStatus() {
    Serial.print("[LORA DEBUG] RSSI: ");
    Serial.print(LoRa.packetRssi());
    Serial.print(" dBm, SNR: ");
    Serial.print(LoRa.packetSnr());
    Serial.println(" dB");
}
```

## Customization

### Adding Additional Sensors
To integrate environmental sensors (temperature, humidity, etc.):

```cpp
#include <DHT.h>
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    dht.begin();
    // ... existing setup
}

void triggerLoRaSend(bool isButtonTriggered) {
    // ... existing GPS code
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    
    gpsData += "TEMP: " + String(temperature) + "\n";
    gpsData += "HUMIDITY: " + String(humidity) + "\n";
    // ... rest of function
}
```

### Implementing Data Logging
Add SD card logging to the receiver:

```cpp
#include <SPI.h>
#include <SD.h>

#define SD_CS 13

void setup() {
    if (!SD.begin(SD_CS)) {
        Serial.println("[SD ERROR] Card initialization failed");
    } else {
        Serial.println("[SD OK] Card initialized");
    }
    // ... existing setup
}

void logGPSData(String data) {
    File file = SD.open("/gps_log.txt", FILE_APPEND);
    if (file) {
        file.println(data);
        file.close();
        Serial.println("[LOG] Data saved to SD card");
    }
}

// In loop():
if (packetSize) {
    // ... existing parsing code
    logGPSData(receivedData);
}
```

### Creating Mobile Integration
Parse GPS data for mobile apps:

```cpp
// Convert to JSON format for mobile apps
String createJSONPacket(float lat, float lng, float alt, String triggerType) {
    String json = "{";
    json += "\"trigger\":\"" + triggerType + "\",";
    json += "\"latitude\":" + String(lat, 6) + ",";
    json += "\"longitude\":" + String(lng, 6) + ",";
    json += "\"altitude\":" + String(alt) + ",";
    json += "\"timestamp\":" + String(millis());
    json += "}";
    return json;
}

// Usage in triggerLoRaSend():
String jsonData = createJSONPacket(
    gps.location.lat(),
    gps.location.lng(),
    gps.altitude.meters(),
    isButtonTriggered ? "BUTTON" : "TIMER"
);
sendPacket(jsonData);
```

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork** the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. Open a **Pull Request**

### Contribution Areas
- **Hardware Improvements**: Antenna designs, power management circuits
- **Software Features**: Data encryption, mobile app integration, cloud connectivity
- **Documentation**: Circuit diagrams, calibration guides, application examples
- **Optimizations**: Power consumption reduction, range extension techniques

### Code Style Guidelines
- Follow Arduino coding conventions
- Use descriptive variable names
- Comment complex algorithms and interrupt handlers
- Test on actual hardware before submitting
- Maintain backward compatibility when possible

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
