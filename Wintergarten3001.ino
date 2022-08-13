/*
 * wire BME280 as follows:
 * Vin 3.3V
 * GND GND
 * SCL GPIO 22
 * SDA GPIO 21
 * 
 * wire relais as follows
 * open window relais - 25
 * close window relais - 26
 * 
 * wire rain sensor as follows
 * bucket contact - 18
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define WINDOW_IGNORE -1
#define WINDOW_OPEN 0
#define WINDOW_CLOSED 1
#define RELAIS_ACTIVE_MS 100

#define PIN_RELAIS_OPEN 25
#define PIN_RELAIS_CLOSE 26
#define PIN_RELAIS_ACTIVE 0
#define PIN_RAIN 18

int newWindowState = WINDOW_IGNORE;
int currentWindowState = WINDOW_CLOSED;
Adafruit_BME280 bme;
float temperature;
float pressure;
float humidity;

void readRainSensor(){}
void readTemperatureSensor(){temperature=bme.readTemperature();}  // in degree C
void readPressureSensor(){pressure=bme.readPressure();}              // in 100th hPa
void readHumiditySensor(){humidity=bme.readHumidity();}        // in Percent
int evaluteWindowPosition(){
  return (millis()/37)%2;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_RAIN,INPUT_PULLUP);
  pinMode(PIN_RELAIS_OPEN,OUTPUT);
  pinMode(PIN_RELAIS_CLOSE,OUTPUT);

  bme.begin(0x76);  

  Serial.println("Wintergarten Steuerung starting");
}

void setWindowState(int newWindowState){
  if (currentWindowState != newWindowState) {
    setOutput(newWindowState);
    currentWindowState = newWindowState;
  } else {
    Serial.println("keeping old state");
  }
  Serial.print("current state ");
  Serial.println(currentWindowState);
}

void setOutput(int relaisState){
  setRelaisState(relaisState);
  delay(RELAIS_ACTIVE_MS);
  setRelaisState(WINDOW_IGNORE);
}

void setRelaisState(int relaisState){
  switch(relaisState){
    case WINDOW_OPEN:
      digitalWrite(PIN_RELAIS_CLOSE,!PIN_RELAIS_ACTIVE);
      digitalWrite(PIN_RELAIS_OPEN,PIN_RELAIS_ACTIVE);
      break;
    case WINDOW_CLOSED:
      digitalWrite(PIN_RELAIS_OPEN,!PIN_RELAIS_ACTIVE);
      digitalWrite(PIN_RELAIS_CLOSE,PIN_RELAIS_ACTIVE);
      break;
    case WINDOW_IGNORE:
      digitalWrite(PIN_RELAIS_CLOSE,!PIN_RELAIS_ACTIVE);
      digitalWrite(PIN_RELAIS_OPEN,!PIN_RELAIS_ACTIVE);
      break;
  }
  Serial.print("relais state ");
  Serial.println(relaisState);
}

void loop() {
  Serial.println("---------");
    readRainSensor();
    readTemperatureSensor();
    readPressureSensor();
    readHumiditySensor();

    Serial.print("Temp ");
    Serial.print(temperature);
    Serial.print("C Hum ");
    Serial.print(humidity);
    Serial.print("% Pres ");
    Serial.print(pressure/100.f);
    Serial.println("hPa");

    newWindowState = evaluteWindowPosition();

    setWindowState(newWindowState);

    delay(3000);
}
