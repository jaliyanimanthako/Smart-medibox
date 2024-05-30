#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "DHTesp.h"

// Pin Definitions
#define LDRPIN1 34  // First LDR pin
#define LDRPIN2 35  // Second LDR pin


// Objects Initialization
Servo servoMotor; // Servo motor object
WiFiClient espClient;
PubSubClient mqttClient(espClient);
DHTesp dhtSensor;

// Constants
const int DHT_PIN = 15;
const float GAMMA = 0.7;
const float RL10 = 50; // Resistance value
const float MIN_LUX = 0.1; // Minimum lux value
const float MAX_LUX = 100000.0; // Maximum lux value

// Variables
int thetaOffset = 30; // Minimum servo motor angle
float gammaValue = 0.75; // Control factor for motor movement

void setup() {
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  setupWifi();
  setupMQTT();
  servoMotor.attach(18); // Attach servo to pin 18
  delay(1000);
}

void loop() {
  if (!mqttClient.connected()) {
    connectToBroker();
  }
  mqttClient.loop();
  
  // Measure light intensity for the first LDR
  float lux1 = measureLightIntensity(LDRPIN1);
  // Measure light intensity for the second LDR
  float lux2 = measureLightIntensity(LDRPIN2);

  // Determine maximum intensity and motor direction
  float maxIntensity;
  float direction;
  if (lux1 >= lux2) {
    maxIntensity = scaleLux(lux1);
    direction = 0.5;
  } else {
    maxIntensity = scaleLux(lux2);
    direction = 1.5;
  }
  
  // Calculate motor angle based on intensity and direction
  int motorAngle = calculateMotorAngle(maxIntensity, direction);
  servoMotor.write(motorAngle); // Rotate the servo to the calculated angle
  
  // Publish lux values
  publishLux("ENTC-LUX1", scaleLux(lux1));
  publishLux("ENTC-LUX2", scaleLux(lux2));

  Serial.print("Gamma: ");
  Serial.println(gammaValue);

  Serial.print("Theta: ");
  Serial.println(thetaOffset);

  updateTempAndHumidity();
  delay(1000);
}

// Function to measure light intensity
float measureLightIntensity(int LDRPIN) {
  int val = analogRead(LDRPIN);
  float voltage = val * (3.3 / 4063.0); // Convert analog reading to voltage
  float resistance = (10000.0 * voltage) / (3.3 - voltage); // Calculate resistance
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1.0 / GAMMA)); // Calculate lux
  return lux;
}

// Function to scale lux value to the range [0, 1]
float scaleLux(float lux) {
  return (lux - MIN_LUX) / (MAX_LUX - MIN_LUX);
}


// Function to calculate motor angle
int calculateMotorAngle(float intensity, float direction) {
  float angle = min(thetaOffset * direction + (180 - thetaOffset) * intensity * gammaValue, 180.0f);
  return int(angle); // Convert float angle to int
}

// Function to setup Wi-Fi connection
void setupWifi() {
  WiFi.begin("Wokwi-GUEST", ""); // Connect to Wi-Fi network

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Function to setup MQTT connection
void setupMQTT() {
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(reciveCallback);
}

// Function to connect to MQTT broker
void connectToBroker() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32-465456646675565")) {
      Serial.println("Connected");
      mqttClient.subscribe("ENTC-SlidingBar");
      mqttClient.subscribe("ENTC-OffsetTheta");
    } else {
      Serial.print("failed ");
      Serial.print(mqttClient.state());
    }
  }
}

// Callback function for receiving MQTT messages
void reciveCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]");

  char payloadCharAr[length];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i];
  }

  if (strcmp(topic, "ENTC-SlidingBar") == 0) {
    // Parse the sliding bar value
    float slidingBarValue = atof(payloadCharAr);
    gammaValue = slidingBarValue;
    Serial.print("Sliding bar value: ");
    Serial.println(slidingBarValue);
  }

  if (strcmp(topic, "ENTC-OffsetTheta") == 0) {
    // Parse the sliding bar value
    float offsetThetaValue = atof(payloadCharAr);
    thetaOffset = offsetThetaValue;
    Serial.print("Sliding bar value: ");
    Serial.println(offsetThetaValue);
  }
}

// Function to publish lux values
void publishLux(const char* topic, float lux) {
  char luxStr[20];
  dtostrf(lux, 10, 5, luxStr);
  mqttClient.publish(topic, luxStr);
}

// Function to update temperature and humidity values
void updateTempAndHumidity() {
  float temperature = dhtSensor.getTemperature();
  float humidity = dhtSensor.getHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Publish temperature and humidity values
  mqttClient.publish("ENTC-Temperature", String(temperature).c_str());
  mqttClient.publish("ENTC-Humidity", String(humidity).c_str());
}
