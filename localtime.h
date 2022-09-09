#ifndef LOCALTIME_H
#define LOCALTIME_H

#include "time.h"

/*
 * time server stuff
 */
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;

/*
 * called in setup function
 */
void setupLocalTime(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

/*
 * update the time
 */
bool updateTime(){
  if(!getLocalTime(&timeinfo)){
    Serial.println("[ERROR] Failed to obtain time");
    return false;
  }

  // fake daytime to test color fading effects
  long v=(millis()/1000)%10;
  timeinfo.tm_min=4+(v>=5);
  timeinfo.tm_sec=(55+v)%60;
  // end of time faking

  return true;
}

/*
 * print local time
 */
void printLocalTime(){
  if (updateTime())Serial.print(&timeinfo, "%H:%M:%S,%d.%m.%y ");
}

/*
 * checks, if convenience opening/closing skylight is allowed {i.e. it is daytime :-)
 */
bool isDrivingAllowed(){
  if(!updateTime())return true;
  if ((timeinfo.tm_hour>=9)&&(timeinfo.tm_hour<21))return true;
  return false;
}

#endif
