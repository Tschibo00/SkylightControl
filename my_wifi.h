#ifndef MY_WIFI_H
#define MY_WIFI_H

#include <WiFi.h>
#include <WiFiClient.h>
#include "secrets.hh"

/*
 * called in setup function
 */
void setupWifi(){
  WiFi.begin(WIFI_SSID,WIFI_PW);
  while (WiFi.status()!=WL_CONNECTED){delay(500);Serial.print(".");}
  Serial.println(WiFi.localIP());
}

#endif
