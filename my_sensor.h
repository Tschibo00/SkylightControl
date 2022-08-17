#ifndef MY_SENSOR_H
#define MY_SENSOR_H

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include "hw_config.h"
#include "config.h"
#include "my_log.h"

Adafruit_BME280 bme;            // sensor library
float temperature;              // environment readings
float pressure;
float humidity;
bool raining=false;

long oldRainBucketCount=0l;     // used to calculate if rain starts falling
long newRainBucketCount=0l;     // increased by interrupt
long rainPinDebounce=0l;
long nextRainAccumulation=0l;
long lastAccumulatedRainValue=0l;
float rainAmount=0.f;
long unlockRainClosure=0l;      // timing variables
bool rainClosureLocked=false;
long rainPerHour;


/*
 * BME280 sensor readout functions
 */
void readTemperatureSensor(){                      // in degree C
  #ifdef DEBUG_LOCAL
  temperature=((float)analogRead(PIN_DEBUG_TEMP))/200.f+20.f;
  #else
  temperature=bme.readTemperature();
  #endif
}

void readPressureSensor(){                         // in hPa
  pressure=bme.readPressure()/100.f;
}

void readHumiditySensor(){                         // in Percent
  #ifdef DEBUG_LOCAL
  humidity=((float)analogRead(PIN_DEBUG_HUM))/41.f;
  #else
  humidity=bme.readHumidity();
  #endif
}

/*
 * interrupt function called when rain sensor triggers
 */
void onRainTrigger(){
  if (millis()>rainPinDebounce){
    newRainBucketCount++;
    rainPinDebounce=millis()+DEBOUNCE_MS;
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
 * called in setup function
 */
void setupSensor(){
  bme.begin(0x76);
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X16, // temperature
                  Adafruit_BME280::SAMPLING_X16, // pressure
                  Adafruit_BME280::SAMPLING_X16, // humidity
                  Adafruit_BME280::FILTER_X16   );
  attachInterrupt(PIN_RAIN, onRainTrigger, FALLING);
}

#endif
