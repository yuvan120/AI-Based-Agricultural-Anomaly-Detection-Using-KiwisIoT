#include <WiFi.h>
#include <Wire.h>
#include <KiwisIoT.h>
#include <DHT.h>
#include <BH1750.h>

// WiFi details
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// KiwisIoT panel topic ID
const char* topic = "YOUR_PANEL_TOPIC_ID";

KiwisIoT kiwisiot(ssid, password, topic);

// Sensor pins
#define SOIL_PIN 34
#define DHT_PIN 4
#define DHTTYPE DHT22

DHT dht(DHT_PIN, DHTTYPE);
BH1750 lightMeter;

// Soil calibration values.
// Replace these after testing your own sensor.
const int DRY_VALUE = 3000;
const int WET_VALUE = 1200;

unsigned long lastSend = 0;
const unsigned long interval = 5000;

void setup() {
  Serial.begin(115200);

  dht.begin();
  Wire.begin(21, 22);
  lightMeter.begin();

  kiwisiot.begin();

  Serial.println("Agricultural Monitoring Started");
}

void loop() {
  kiwisiot.run();

  if (millis() - lastSend >= interval) {
    lastSend = millis();

    // Read soil moisture
    int soilRaw = analogRead(SOIL_PIN);

    int soil = map(
      soilRaw,
      DRY_VALUE,
      WET_VALUE,
      0,
      100
    );

    soil = constrain(soil, 0, 100);

    // Read temperature and humidity
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Read light intensity
    float light = lightMeter.readLightLevel();

    // Check sensor validity
    if (isnan(temperature) ||
        isnan(humidity) ||
        light < 0) {
      Serial.println("Sensor reading error");
      return;
    }

    // Anomaly detection
    bool anomaly = false;

    if (soil < 20) {
      anomaly = true;
      Serial.println("ALERT: Soil is too dry");
    }

    if (temperature > 40) {
      anomaly = true;
      Serial.println("ALERT: High temperature");
    }

    if (humidity < 30) {
      anomaly = true;
      Serial.println("ALERT: Low humidity");
    }

    // Send data to KiwisIoT
    kiwisiot.send("0", soil);
    kiwisiot.send("1", temperature);
    kiwisiot.send("2", humidity);
    kiwisiot.send("6", light);
    kiwisiot.send("7", anomaly ? 1 : 0);

    // Serial output
    Serial.println("---------------------");
    Serial.print("Soil Moisture: ");
    Serial.print(soil);
    Serial.println("%");

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");

    Serial.print("Light: ");
    Serial.print(light);
    Serial.println(" lux");

    Serial.print("Anomaly: ");
    Serial.println(anomaly ? "DETECTED" : "NORMAL");

    Serial.println("---------------------");
  }
}