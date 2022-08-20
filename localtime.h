#ifndef LOCALTIME_H
#define LOCALTIME_H

#include "time.h"

/*
 * time server stuff
 */
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

/*
 * called in setup function
 */
void setupLocalTime(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

/*
 * print local time
 */
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("[ERROR] Failed to obtain time");
    return;
  }
  Serial.print(&timeinfo, "%H:%M:%S,%d.%m.%y ");
}

/*
 * checks, if convenience opening/closing skylight is allowed {i.e. it is daytime :-)
 */
bool isDrivingAllowed(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("[ERROR] Failed to obtain time");
    return true;
  }
  if ((timeinfo.tm_hour>=9)&&(timeinfo.tm_hour<21))return true;
  return false;
}

#endif
