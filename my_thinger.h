#ifndef MY_THINGER_H
#define MY_THINGER_H

#include <ThingerESP32.h>
#include "secrets.hh"

ThingerESP32 thing(THINGER_USER, THINGER_ID, THINGER_TOKEN);

/*
 * called in setup function
 */
void setupThinger(){
  thing.add_wifi(WIFI_SSID, WIFI_PW);
  thing["temperature"] >> outputValue(temperature);
  thing["humidity"] >> outputValue(humidity);
  thing["pressure"] >> outputValue(pressure);
  thing["rain"] >> outputValue(rainAmount);
  thing["light"] >> outputValue(medLightLevel);
}

void thingerHandle(){
  thing.handle();
}

#endif
