#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#define WINDOW_IGNORE -1
#define WINDOW_OPEN 0
#define WINDOW_CLOSED 1

/*
 * HW parameters
 */
#define RELAIS_ACTIVE_MS 100    // switch cycle for the relais
#define PIN_RELAIS_OPEN 25      // relais to open the skylight
#define PIN_RELAIS_CLOSE 26     // relais to close the skylight
#define PIN_RELAIS_ACTIVE 0     // 0 ACTIVE low, 1 ACTIVE high
#define PIN_RAIN 18             // bucket simulation input pin (switch to ground, pulled-up internally)
#define PIN_DEBUG_TEMP 34       // pin to connect temp pot for debugging (define DEBUG_LOCAL to enable)
#define PIN_DEBUG_HUM 35        // pin to connect humidity pot for debugging (define DEBUG_LOCAL to enable)
#define PIN_LIGHT_SENSOR 33     // pin to connect to anode of light sensing LED (cathode goes to ground)

/*
 * called in setup function
 */
void setupPins(){
  pinMode(PIN_RAIN,INPUT_PULLUP);
  pinMode(PIN_RELAIS_OPEN,OUTPUT);
  pinMode(PIN_RELAIS_CLOSE,OUTPUT);
  #ifdef DEBUG_LOCAL
  pinMode(PIN_DEBUG_TEMP,INPUT);
  pinMode(PIN_DEBUG_HUM,INPUT);
  #endif
}

#endif
