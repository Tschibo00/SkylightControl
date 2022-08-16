#ifndef OUTPUT_H
#define OUTPUT_H

#include "hw_config.h"

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

/*
 * called in setup function
 */
void setupOutput(){
  setRelaisState(WINDOW_IGNORE);
}

/*
 * Drives the relais based on the required state
 */
void setOutput(int relaisState){
  setRelaisState(relaisState);
  delay(RELAIS_ACTIVE_MS);
  setRelaisState(WINDOW_IGNORE);
}

#endif
