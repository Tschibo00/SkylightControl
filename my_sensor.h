#ifndef MY_SENSOR_H
#define MY_SENSOR_H

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include "hw_config.h"
#include "config.h"
#include "my_log.h"
#include "output.h"

Adafruit_BME280 bme;            // sensor library
float temperature=30.f;         // environment readings
float pressure=940.f;
float humidity=40.f;
bool raining=false;

bool isSensorValid=true;

long oldRainBucketCount=0l;     // used to calculate if rain starts falling
long newRainBucketCount=0l;     // increased by interrupt
long rainPinDebounce=0l;
long nextRainAccumulation=0l;
long lastAccumulatedRainValue=0l;
float rainAmount=0.f;
long unlockRainClosure=0l;      // timing variables
bool rainClosureLocked=false;
long rainPerHour;

long lastLightSensorReading=0L;      // light sensor values
int lightLevelRaw[10];              // raw value from analogRead
uint8_t lightArrayPtr=0;
float lightLevelFloat;          // corrected values (0.0-1.0)
float medLightLevel;

/*
 * Reads analog value of light sensor (every second to avoid lag)
 */
float getLightSensorReading(){
  if (millis()>lastLightSensorReading+1000){
    lightLevelRaw[lightArrayPtr]=analogRead(PIN_LIGHT_SENSOR);
    lightArrayPtr=(lightArrayPtr+1)%10;
    medLightLevel=0.f;
    for (uint8_t i=0;i<10;i++)
      medLightLevel+=lightLevelRaw[i];
    medLightLevel=medLightLevel/10.f;
    lightLevelFloat=((float)(medLightLevel-LIGHT_LOWEST))/((float)(LIGHT_HIGHEST-LIGHT_LOWEST));
    if (lightLevelFloat<0.f) lightLevelFloat=0.f;
    if (lightLevelFloat>1.f) lightLevelFloat=1.f;
    lightLevelFloat=pow(lightLevelFloat,LIGHT_EXPONENT);
    lastLightSensorReading=millis();
  }
  return lightLevelFloat;
}

/*
 * converts light level to luminance value for display
 */
int getBrightnessFromLightLevel(){
  float range=LUM_HIGHEST-LUM_LOWEST;
  int val=getLightSensorReading()*range+LUM_LOWEST;
  if (val<0)val=0;
  if (val>255)val=255;
  return val;
}
 
/*
 * BME280 sensor readout functions
 */
void readTemperatureSensor(){                      // in degree C
  #ifdef DEBUG_LOCAL
  temperature=((float)analogRead(PIN_DEBUG_TEMP))/200.f+20.f;
  #else
  if (isSensorValid)temperature=bme.readTemperature()-getLightSensorReading()*TEMP_LUM_CORRECTION;
  #endif
}

void readPressureSensor(){                         // in hPa
  if (isSensorValid)pressure=bme.readPressure()/100.f;
}

void readHumiditySensor(){                         // in Percent
  #ifdef DEBUG_LOCAL
  humidity=((float)analogRead(PIN_DEBUG_HUM))/41.f;
  #else
  if (isSensorValid)humidity=bme.readHumidity();
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
    oldRainBucketCount=newRainBucketCount;      // reset counter to remove false positives
  }
}

/*
 * called in setup function
 */
void setupSensor(){
  unsigned status=bme.begin(0x76);
  if (!status){
    Serial.println("Could not find a valid BME280 sensor");
    isSensorValid=false;
  }
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X16, // temperature
                  Adafruit_BME280::SAMPLING_X16, // pressure
                  Adafruit_BME280::SAMPLING_X16, // humidity
                  Adafruit_BME280::FILTER_X16   );
  attachInterrupt(PIN_RAIN, onRainTrigger, FALLING);
}

#endif
