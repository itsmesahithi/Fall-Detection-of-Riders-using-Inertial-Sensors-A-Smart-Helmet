#include <Wire.h>
#include <Adafruit_ADXL345_U.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Define sensor pins
#define IR_SENSOR 2
#define TRIG_PIN 3
#define ECHO_PIN 4
#define ALCOHOL_SENSOR A0
#define GREEN_LED 6
#define RED_LED 7
#define BLINKING_LED 8
#define RELAY 9

// GPS Module Pins
#define RX_GPS 10
#define TX_GPS 11

// Create objects for GPS and Accelerometer
SoftwareSerial gpsSerial(RX_GPS, TX_GPS);
TinyGPSPlus gps;
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(123);

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);

  pinMode(IR_SENSOR, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(ALCOHOL_SENSOR, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLINKING_LED, OUTPUT);
  pinMode(RELAY, OUTPUT);

  if (!accel.begin()) {
    Serial.println("Could not find ADXL345 sensor!");
    while (1);
  }
}

void loop() {
  int alcoholLevel = analogRead(ALCOHOL_SENSOR);
  bool helmetWorn = digitalRead(IR_SENSOR) == LOW;

  sensors_event_t event;
  accel.getEvent(&event);
  float acceleration = sqrt(event.acceleration.x * event.acceleration.x +
                            event.acceleration.y * event.acceleration.y +
                            event.acceleration.z * event.acceleration.z);

  // Helmet Detection
  if (helmetWorn) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RELAY, HIGH);  // Allow bike to start
    Serial.println("Helmet detected, bike can start.");
  } else {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RELAY, LOW);
    Serial.println("Helmet not detected. Ignition blocked.");
  }

  // Alcohol Detection
  if (alcoholLevel > 300) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(RELAY, LOW);
    Serial.println("Alcohol detected! Ignition blocked.");
  } else {
    digitalWrite(RED_LED, LOW);
  }

  // Accident Detection
  if (acceleration > 10.0) {  // Adjust threshold if needed
    Serial.println("⚠️ Accident Detected! Fetching GPS Location...");
    digitalWrite(BLINKING_LED, HIGH);
    getGPSLocation();
    delay(5000);
    digitalWrite(BLINKING_LED, LOW);
  }

  delay(500);
}

// Function to get GPS location
void getGPSLocation() {
  String gpsData = "Location: ";

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
    if (gps.location.isUpdated()) {
      gpsData += String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
      Serial.println(gpsData);
      break;
    }
  }
}
