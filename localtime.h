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
  Serial.print(millis()%1000);
  Serial.print(" ");
  Serial.print(&timeinfo, "%H:%M:%S,%d.%m.%y ");
}

#endif
