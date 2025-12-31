#include <SPI.h>
#include <LoRa.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>

// =========================================================================
// PIN DEFINITIONS
// =========================================================================

// ---- LoRa (esp32 devkit v4)
#define LORA_SS 5
#define LORA_RST 22
#define LORA_DIO0 21
// GPS Serial Pins
#define RX 16
#define TX 17
#define GPS_BAUD 9600

// Push Button Interrupt Pin
#define BUTTON_PIN 4

// =========================================================================
// INTERRUPT & TIMING VARIABLES
// =========================================================================
// This flag is set by the ISR and read by the main loop. MUST be volatile.
volatile bool buttonPressed = false;
// Debounce time for the ISR (in milliseconds). MUST be volatile.
volatile unsigned long lastButtonInterruptTime = 0;
#define ISR_DEBOUNCE_DELAY 200 // Minimum time between button presses

// =========================================================================
// INSTANCES AND FUNCTIONS
// =========================================================================

TinyGPSPlus gps;
// Create an instance of the HardwareSerial class for Serial 2 (ESP32)
HardwareSerial gpsSerial(2);

// Forward declarations
void sendPacket(String message);
void triggerLoRaSend(bool isButtonTriggered);

// =========================================================================
// INTERRUPT SERVICE ROUTINE (ISR) - Must be fast and simple
// =========================================================================
// IRAM_ATTR places the function in IRAM for fast and reliable execution.
void IRAM_ATTR buttonISR() {
    unsigned long currentInterruptTime = millis();

    // 1. Debouncing Check: Ensure enough time has passed since the last trigger
    if (currentInterruptTime - lastButtonInterruptTime > ISR_DEBOUNCE_DELAY) {
        // 2. State Check: Only trigger if the button state is LOW (pressed, due to INPUT_PULLUP)
        if (digitalRead(BUTTON_PIN) == LOW) {
            buttonPressed = true;
            lastButtonInterruptTime = currentInterruptTime; // Update timestamp
        }
    }
}


void setup() {
    Serial.begin(115200);

    // --- LoRa Setup ---
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
    if (!LoRa.begin(433E6)) {
        Serial.println("[FAIL] LoRa init failed! Check wiring and frequency.");
        while (1);
    }
    LoRa.setTxPower(55);
    LoRa.setSpreadingFactor(10);
    Serial.println("[OK] LoRa Initialized");

    // --- GPS Setup ---
    // Start Serial 2 with the defined RX and TX pins and a baud rate of 9600 (ESP32)
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RX, TX);
    Serial.println("[OK] GPS Serial 2 Initialized");

    // --- Push Button Setup ---
    // Set button pin as input with internal pull-up resistor
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Attach the interrupt to the button pin
    // FALLING means trigger when pin goes from HIGH (released) to LOW (pressed)
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

    Serial.print("[OK] Interrupt Attached to GPIO ");
    Serial.println(BUTTON_PIN);
    Serial.println("----------------------------");
}


// Function to acquire GPS data and send it via LoRa
void triggerLoRaSend(bool isButtonTriggered) {
    // String to hold all concatenated GPS data lines
    String gpsData = "";
    bool fixFound = false;

    // Read GPS data for up to 1 second
    unsigned long start = millis();
    while (millis() - start < 1000) {
        while (gpsSerial.available() > 0) {
            gps.encode(gpsSerial.read());
        }
        if (gps.location.isUpdated()) {
            fixFound = true;
            break; // Exit early once a location update is found
        }
    }

    // Determine the trigger type string
    String triggerType = isButtonTriggered ? "BUTTON" : "TIMER";

    if (fixFound) {
        // Assemble the full GPS data string
        gpsData += "--- LoRa GPS Packet ---\n";
        gpsData += "TRIGGER_TYPE: " + triggerType + "\n"; // Explicit trigger type included

        gpsData += "LAT: " + String(gps.location.lat(), 6) + "\n";
        gpsData += "LONG: " + String(gps.location.lng(), 6) + "\n";
        gpsData += "ALT: " + String(gps.altitude.meters()) + "\n";
        gpsData += "Time (UTC): " + String(gps.date.year()) + "/" + String(gps.date.month()) + "/" + String(gps.date.day()) + "," + String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()) + "\n";

        // Send the entire accumulated string as one packet
        sendPacket(gpsData);

        Serial.println(gpsData);
        Serial.println("----------------------------");

    } else {
        Serial.print("[WARNING] No valid GPS fix found to send (");
        Serial.print(triggerType);
        Serial.println(" trigger)");
    }
}


void loop() {
    unsigned long currentTime = millis();

    // =========================================================================
    // 1. INTERRUPT HANDLER: Check the volatile flag
    // =========================================================================
    if (buttonPressed) {

        // Critical section: Temporarily disable interrupts while clearing the flag
        // to prevent the ISR from running simultaneously.
        portDISABLE_INTERRUPTS();
        buttonPressed = false;
        portENABLE_INTERRUPTS();

        // Perform the lengthy LoRa/GPS operation outside the ISR
        Serial.println("[INTERRUPT] Triggering immediate LoRa transmission.");
        triggerLoRaSend(true); // true = button triggered
    }

    // =========================================================================
    // 2. CONTINUOUS GPS FEEDING (for up-to-date data)
    // =========================================================================
    // Even when not sending, we must continuously feed the TinyGPS++ library.
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    // =========================================================================
    // 3. TIMER-BASED TRANSMISSION (Optional, based on your original loop)
    // =========================================================================
    static unsigned long lastTransmitTime = 0;
    const unsigned long TRANSMIT_INTERVAL = 60000; // Example: Send every 60 seconds

    if (currentTime - lastTransmitTime >= TRANSMIT_INTERVAL) {
        Serial.println("[TIMER] Triggering scheduled LoRa transmission.");
        triggerLoRaSend(false); // false = timer triggered
        lastTransmitTime = currentTime;
    }

    delay(10);
}


void sendPacket(String message){
    // Send via LoRa
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();

    Serial.println("Packet Transmitted via LoRa");
}