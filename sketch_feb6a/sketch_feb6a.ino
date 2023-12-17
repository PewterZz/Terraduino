#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <FastLED.h>
#include <DHT.h>

#define LED_PIN         23
#define COLOR_ORDER     GRB
#define CHIPSET         WS2812
#define NUM_LEDS        24
#define BRIGHTNESS      150
#define FRAMES_PER_SECOND 60
CRGB leds[NUM_LEDS];

#define COOLING         55
#define SPARKING        120

// Pin definitions
const int soilMoisturePin = 19;
const int dhtPin = 32;           // Digital pin for DHT humidity sensor
const int in1Pin = 5;           // Digital pin for H-bridge IN1
const int in2Pin = 2;           // Digital pin for H-bridge IN2
const int in3Pin = 26;           // Digital pin for H-bridge IN3
const int in4Pin = 25;           // Digital pin for H-bridge IN4
const int power = 18;           // Digital pin for H-bridge ENA;          // Digital pin for H-bridge ENB

// Create instances of DHT sensor
DHT dht(dhtPin, DHT11);

// BLE settings
BLEServer* pServer = NULL;
BLEService* pService = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

float soilMoisture = 0.0;
float temperature = 0.0;
float humidity = 0.0;

bool hBridgeOn = false;
bool enAActive = true; // Initial state, activate ENA

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("Botanik_Justin");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  pService = pServer->createService("19b10000-e8f2-537e-4f6c-d104768a1214");
  pCharacteristic = pService->createCharacteristic("19b10000-e8f2-537e-4f6c-d104768a1214", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pService->start();

  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(soilMoisturePin, INPUT); 
  dht.begin();

  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
  pinMode(power, OUTPUT);
  pinMode(14 , OUTPUT);

  digitalWrite(14, LOW);
  digitalWrite(power, HIGH);
}

void updateSensors() {
  soilMoisture = analogRead(soilMoisturePin);
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  Serial.println(humidity);
}

void setBrightness(int brightness) {
  FastLED.setBrightness(brightness);
}

void Fire2012() {
  static bool isRed = true; // Flag to track color state
  static unsigned long lastChange = millis(); // Initialize to current time
  unsigned long now = millis(); // Current time
  unsigned long delayTime = 30000; // Delay time in milliseconds

  // Check if it's time to change the color
  if (now - lastChange > delayTime) {
    isRed = !isRed; // Toggle the color state
    lastChange = now; // Update the last change time

    // Set all LEDs to the new color
    for (int i = 0; i < NUM_LEDS; i++) {
      if (isRed) {
        leds[i] = CRGB::Red; // Set LED to red
      } else {
        leds[i] = CRGB::Blue; // Set LED to blue
      }
    }
  }
}

void loop() {
  if (deviceConnected) {
    updateSensors();
    std::string value = pCharacteristic->getValue();
    if (!value.empty()) {
      if (value == "SET_BRIGHTNESS") {
        uint8_t brightnessValue = value[0];
        setBrightness(brightnessValue);
      } else if (value == "TURN_ON_PUMP") {
          digitalWrite(in1Pin, LOW);
          digitalWrite(in2Pin, LOW);

          delay(1000);

          digitalWrite(in3Pin, HIGH);
          digitalWrite(in4Pin, LOW);
      }
      else if (value == "TURN_OFF_PUMP") {
          digitalWrite(in1Pin, LOW);
          digitalWrite(in2Pin, LOW);
          digitalWrite(in3Pin, LOW);
          digitalWrite(in4Pin, LOW);
      }
      else if (value == "TURN_ON_LIGHT") {
          digitalWrite(in3Pin, LOW);
          digitalWrite(in4Pin, LOW);

          delay(1000);

          digitalWrite(in1Pin, HIGH);
          digitalWrite(in2Pin, LOW);
      } else if (value == "TURN_OFF_LIGHT") {
          digitalWrite(in3Pin, LOW);
          digitalWrite(in4Pin, LOW);
          digitalWrite(in1Pin, LOW);
          digitalWrite(in2Pin, LOW);
      }
      else if (value == "TURN_BRIDGE_OFF") {
          digitalWrite(in1Pin, LOW);
          digitalWrite(in2Pin, LOW);
          digitalWrite(in3Pin, LOW);
          digitalWrite(in4Pin, LOW);
      }
    }

    std::string response = "Soil Moisture: " + std::to_string(soilMoisture) + "\n";
    response += "Temperature: " + std::to_string(temperature) + "\n";
    response += "Humidity: " + std::to_string(humidity) + "\n";
    pCharacteristic->setValue(response);
    pCharacteristic->notify();

  }

  Fire2012();
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}
