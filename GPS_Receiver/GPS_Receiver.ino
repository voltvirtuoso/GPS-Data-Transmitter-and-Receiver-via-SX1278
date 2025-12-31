#include <SPI.h>
#include <LoRa.h>

// =========================================================================
// PIN DEFINITIONS (Same as transmitter)
// =========================================================================
// ---- LoRa (esp32 devkit v4) - Same pins as transmitter
#define LORA_SS 5
#define LORA_RST 22
#define LORA_DIO0 21

void parseGPSData(String packet);

// =========================================================================
// SETUP FUNCTION
// =========================================================================
void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("LoRa GPS Receiver");
    Serial.println("=================");

    // --- LoRa Setup (Same configuration as transmitter) ---
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("[FAIL] LoRa init failed! Check wiring and frequency.");
        while (1);
    }
    LoRa.setTxPower(55);
    LoRa.setSpreadingFactor(10);
    Serial.println("[OK] LoRa Initialized");
    Serial.println("Waiting for packets...");
    Serial.println("----------------------------");
}

// =========================================================================
// LOOP FUNCTION - Continuously listen for LoRa packets
// =========================================================================
void loop() {
    // Try to parse the packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        Serial.print("[RECEIVED] Packet size: ");
        Serial.print(packetSize);
        Serial.println(" bytes");
        Serial.println("----------------------------");

        // Read packet and print to Serial
        String receivedData = "";
        while (LoRa.available()) {
            receivedData += (char)LoRa.read();
        }

        // Print the complete received packet
        Serial.println(receivedData);
        
        // Parse GPS data from the received packet
        parseGPSData(receivedData);
        
        Serial.println("----------------------------");
    }
    
    delay(10); // Small delay to prevent watchdog trigger
}

// =========================================================================
// GPS DATA PARSER FUNCTION
// =========================================================================
void parseGPSData(String packet) {
    // Extract relevant information from the packet
    if (packet.indexOf("LAT:") != -1 && packet.indexOf("LONG:") != -1) {
        Serial.println("[GPS DATA PARSED]");
        
        // Extract trigger type
        int triggerStart = packet.indexOf("TRIGGER_TYPE: ");
        if (triggerStart != -1) {
            int triggerEnd = packet.indexOf("\n", triggerStart);
            String triggerType = packet.substring(triggerStart + 14, triggerEnd);
            Serial.print("Trigger Type: ");
            Serial.println(triggerType);
        }
        
        // Extract latitude
        int latStart = packet.indexOf("LAT: ");
        if (latStart != -1) {
            int latEnd = packet.indexOf("\n", latStart);
            String latStr = packet.substring(latStart + 5, latEnd);
            Serial.print("Latitude: ");
            Serial.println(latStr);
        }
        
        // Extract longitude
        int longStart = packet.indexOf("LONG: ");
        if (longStart != -1) {
            int longEnd = packet.indexOf("\n", longStart);
            String longStr = packet.substring(longStart + 6, longEnd);
            Serial.print("Longitude: ");
            Serial.println(longStr);
        }
        
        // Extract altitude
        int altStart = packet.indexOf("ALT: ");
        if (altStart != -1) {
            int altEnd = packet.indexOf("\n", altStart);
            String altStr = packet.substring(altStart + 5, altEnd);
            Serial.print("Altitude: ");
            Serial.print(altStr);
            Serial.println(" meters");
        }
        
        // Extract UTC time
        int timeStart = packet.indexOf("Time (UTC): ");
        if (timeStart != -1) {
            int timeEnd = packet.indexOf("\n", timeStart);
            String timeStr = packet.substring(timeStart + 12, timeEnd);
            Serial.print("UTC Time: ");
            Serial.println(timeStr);
        }
    } else {
        Serial.println("[WARNING] No valid GPS data found in packet");
    }
}