#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Pin Definitions untuk HC-SR04
const int TRIG_PIN = 5;
const int ECHO_PIN = 18;

// WiFi Credentials (untuk Wokwi)
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT Configuration
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Variables
unsigned long lastMsg = 0;
float distance = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Subscribe ke topik jika diperlukan
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

float readDistance() {
  // Clear trig pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Set trig pin HIGH for 10 microseconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read echo pin
  long duration = pulseIn(ECHO_PIN, HIGH);
  
  // Calculate distance (cm)
  float distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  Serial.begin(115200);
  
  // Initialize sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) { // Kirim data setiap 2 detik
    lastMsg = now;
    
    // Read distance from sensor
    distance = readDistance();
    
    // Publish to MQTT
    char distString[8];
    dtostrf(distance, 1, 2, distString);
    
    client.publish("/cyberacademy/distance", distString);
    
    // Print to Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    
    // Debug: Print di Serial jika ada masalah
    if (distance <= 0 || distance > 400) {
      Serial.println("Invalid distance reading!");
    }
  }
}