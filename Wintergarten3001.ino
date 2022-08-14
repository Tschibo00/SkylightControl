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

/*
 * TODOs
 * hysteresis for humidity
 * webserver for getting and setting the config values
 * rain instantly closes window
 * HW check
 * pulse frequency of rain sensor
 * do double signals in one direction trigger reverse driving of skylight?
 */

#define THINGER_SERIAL_DEBUG


#include <ThingerESP32.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "secrets.h"

#define WINDOW_IGNORE -1
#define WINDOW_OPEN 0
#define WINDOW_CLOSED 1

/*
 * HW parameters
 */
#define RELAIS_ACTIVE_MS 100    // switch cycle for the relais
#define POLL_CYCLE_MS 3000      // cycle time at which conditions are evaluated. Must be longer than the driving time of the skylight!!!
#define PIN_RELAIS_OPEN 25      // relais to open the skylight
#define PIN_RELAIS_CLOSE 26     // relais to close the skylight
#define PIN_RELAIS_ACTIVE 0     // 0 ACTIVE low, 1 ACTIVE high
#define PIN_RAIN 18             // bucket simulation input pin (switch to ground, pulled-up internally)
#define DEBOUNCE_MS 50          // Debounce period
#define RAIN_PER_SIGNAL 0.01f   // l/m2 per rain sensor signal

/*
 * Control parameters
 */
#define TEMP_CLOSE_BELOW 25.f   // close the skylight below this temperature
#define TEMP_OPEN_ABOVE 35.f    // open the skylight above this temperature
#define HUM_OPEN_ABOVE 75.f     // open the skylight above this humidity
#define RAIN_LOCK_MS 600000     // time after rain detection in which convenience opening is disabled
#define RAIN_THRESHOLD 1        // threshold at which the rain counter at least has to change in the last cycle to trigger rain detection
#define RAIN_PERIOD 60000       // period in which rain amount is accumulated (used for pushing to thinger.io)
 
Adafruit_BME280 bme;            // sensor library
float temperature;              // environment readings
float pressure;
float humidity;
bool raining=false;
long rainPerHour;

int currentWindowState = WINDOW_CLOSED;
long oldRainBucketCount=0l;     // used to calculate if rain starts falling
long newRainBucketCount=0l;     // increased by interrupt
long unlockRainClosure=0l;      // timing variables
long rainPinDebounce=0l;
long nextSensorReadout=0l;
long nextRainAccumulation=0l;
long lastAccumulatedRainValue=0l;
float rainAmount=0.f;

ThingerESP32 thing(THINGER_USER, THINGER_ID, THINGER_TOKEN);

void setup() {
  Serial.begin(115200);

  pinMode(PIN_RAIN,INPUT_PULLUP);
  pinMode(PIN_RELAIS_OPEN,OUTPUT);
  pinMode(PIN_RELAIS_CLOSE,OUTPUT);

  thing.add_wifi(WIFI_SSID, WIFI_PW);
  thing["temperature"] >> outputValue(temperature);
  thing["humidity"] >> outputValue(humidity);
  thing["pressure"] >> outputValue(pressure);
  thing["rain"] >> outputValue(rainAmount);

  bme.begin(0x76);

  attachInterrupt(PIN_RAIN, onRainTrigger, FALLING);

  Serial.println("Wintergarten Steuerung starting");
}

/*
 * BME280 sensor readout functions
 */
void readTemperatureSensor(){temperature=bme.readTemperature();}  // in degree C
void readPressureSensor(){pressure=bme.readPressure()/100.f;}     // in hPa
void readHumiditySensor(){humidity=bme.readHumidity();}           // in Percent

void onRainTrigger(){
  if (millis()>rainPinDebounce){
    newRainBucketCount++;
    rainPinDebounce=millis()+DEBOUNCE_MS;
  }
}

/*
 * Signals rain if a certain threshold is passed
 */
void readRainSensor(){
  if (newRainBucketCount>=oldRainBucketCount+RAIN_THRESHOLD){
    oldRainBucketCount=newRainBucketCount;
    raining=true;
  }else{
    raining=false;
  }
}

/*
 * Calculates accumulated amount of rain in given period
 */
void calculateRainAmount(){
  if (millis()>=nextRainAccumulation){
    nextRainAccumulation=millis()+RAIN_PERIOD;
    rainAmount=(newRainBucketCount-lastAccumulatedRainValue)*RAIN_PER_SIGNAL;
    lastAccumulatedRainValue=newRainBucketCount;
  }
}

/*
 * Business logic. Determines, based on environment parameters, if the skylight needs to be open or closed
 */
void evaluteWindowPosition(){
  int newState=WINDOW_IGNORE;
  bool force=false;

  if (millis()>unlockRainClosure){
    // convenience open/close
    if (temperature<TEMP_CLOSE_BELOW){newState=WINDOW_CLOSED;Serial.println("DRIVE: Closing b/c temp too low");}
    if (temperature>TEMP_OPEN_ABOVE){newState=WINDOW_OPEN;Serial.println("DRIVE: Opening b/c temp too high");}
    if (humidity>HUM_OPEN_ABOVE){newState=WINDOW_OPEN;Serial.println("DRIVE: Opening b/c humidity too high");}
    // force close if rain starts
    if (raining){
      Serial.println("DRIVE: Forcibly closing b/c it's raining");
      newState=WINDOW_CLOSED;
      force=true;
    }
  }

  if (raining){
    unlockRainClosure=millis()+RAIN_LOCK_MS;    // extend lock time if it's still raining
  }

  setWindowState(newState,force);
}

/*
 * triggers the new window state (if required)
 * if force is true, the relais are triggered, independent of current state (to forcibly close window, even if the stored state is incorrect, e.g. because the skylight was operated manually)
 */
void setWindowState(int newWindowState,bool force){
  if ((currentWindowState != newWindowState)||force) {
    setOutput(newWindowState);
    currentWindowState = newWindowState;
  }
}

/*
 * Drives the relais based on the required state
 */
void setOutput(int relaisState){
  setRelaisState(relaisState);
  delay(RELAIS_ACTIVE_MS);
  setRelaisState(WINDOW_IGNORE);
}

/*
 * hardware driver for the relais
 */
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
}

void loop() {
  if (millis()>=nextSensorReadout){
    nextSensorReadout=millis()+POLL_CYCLE_MS;
  
    readRainSensor();
    readTemperatureSensor();
    readPressureSensor();
    readHumiditySensor();
    calculateRainAmount();
  
    Serial.print("Rain ");
    Serial.print(raining);
    Serial.print(" (");
    Serial.print(newRainBucketCount);
    Serial.print(") ");
    Serial.print(temperature);
    Serial.print("C ");
    Serial.print(humidity);
    Serial.print("% ");
    Serial.print(pressure);
    Serial.println("hPa");
  
    evaluteWindowPosition();
  }

  thing.handle();
}
