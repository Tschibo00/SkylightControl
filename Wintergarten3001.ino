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
 * 
 * For local debugging attach pots to 34+35 for temperature and humidity
 * define DEBUG_LOCAL to enable local debugging via pots
 */

/*
 * TODOs HW check
 * do double signals in one direction trigger reverse driving of skylight?
 */

//#define THINGER_SERIAL_DEBUG
//#define DEBUG_LOCAL

#include <Arduino.h>

#include "localtime.h"
#include "hw_config.h"
#include "my_state.h"
#include "config.h"
#include "my_sensor.h"
#include "output.h"
#include "my_server.h"
#include "my_wifi.h"
#include "my_thinger.h"
#include "my_log.h"
#include "DisplayController.h"

long nextSensorReadout=0l;

void setup() {
  Serial.begin(115200);

  setupPins();
  setupOutput();
  setupWifi();
  setupThinger();
  setupSensor();
  setupServer();
  setupLocalTime();
  setupConfig();

  initDisplayController();
  
  logln("Wintergarten Steuerung starting");
}

void loop() {

  readRainSensor();
  if (millis()>=nextSensorReadout){
    nextSensorReadout=millis()+POLL_CYCLE_MS;
  
    readTemperatureSensor();
    readPressureSensor();
    readHumiditySensor();
    calculateRainAmount();
  
    log("Rain ");
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
/*
  if (millis()%100==0)
  Serial.println(analogRead(33)); // reads values between 0 (cellar stairs in bright daylight) and around 2100 in sunlight
*/
// TODO move to separate file
// TODO add refreshing the display only if required
// TODO add luminance control
// TODO add temperature compensation, based on luminance (maybe)
  clear(CRGB::Black);
  if (timeinfo.tm_hour>9)
    showDigit37(timeinfo.tm_hour/10, CRGB::Red,0);
  showDigit37(timeinfo.tm_hour%10, CRGB::Red,4);
  showDigit37(timeinfo.tm_min/10, CRGB::Red,9);
  showDigit37(timeinfo.tm_min%10, CRGB::Red,13);
  if ((millis()/500)%2==0){
    set(7,3,CRGB::DarkGray);
    set(8,3,CRGB::DarkGray);
    set(7,5,CRGB::DarkGray);
    set(8,5,CRGB::DarkGray);
  }
  displayShow();

  thingerHandle();
  serverHandle();
}
