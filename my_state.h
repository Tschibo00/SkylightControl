#ifndef MY_STATE_H
#define MY_STATE_H

#include "my_sensor.h"
#include "output.h"
#include "config.h"
#include "my_log.h"

bool humidityLocked=false;
bool tempAboveLocked=false;
bool tempBelowLocked=false;
bool nightClosingDone=false;

/*
 * triggers the new window state (if required)
 * if force is true, the relais are triggered, independent of current state (to forcibly close window, even if the stored state is incorrect, e.g. because the skylight was operated manually)
 */
void setWindowState(int newWindowState,bool force){
  if (newWindowState==WINDOW_IGNORE)return;
  if ((currentWindowState != newWindowState)||force) {
    setOutput(newWindowState);
    currentWindowState = newWindowState;
    writeWindowState();
  }
}

/*
 * Signals rain if a certain threshold is passed
 */
void readRainSensor(){
  if (newRainBucketCount>=oldRainBucketCount+RAIN_THRESHOLD){
    oldRainBucketCount=newRainBucketCount;
    raining=true;

    // instantly force close if rain starts
    if (isDrivingAllowed()){
      if (!rainClosureLocked){
        logln("DRIVE: Forcibly closing b/c it's raining");
        setWindowState(WINDOW_CLOSED,true);
      }
    }
    unlockRainClosure=millis()+RAIN_LOCK_MS;    // extend lock time if it's still raining
    rainClosureLocked=true;
  }else{
    raining=false;
  }
}

/*
 * Business logic. Determines, based on environment parameters, if the skylight needs to be open or closed
 */
void evaluteWindowPosition(){
  int newState=WINDOW_IGNORE;

  if (isDrivingAllowed()){
    nightClosingDone=false;
    if (millis()>unlockRainClosure){
      if (rainClosureLocked){
        rainClosureLocked=false;
        tempBelowLocked=false;
        tempAboveLocked=false;
        humidityLocked=false;
      }
    }
    if (!rainClosureLocked){
      // convenience open/close
      if (temperature<TEMP_CLOSE_BELOW){
        if (!tempBelowLocked){
          newState=WINDOW_CLOSED;
          tempBelowLocked=true;
          tempAboveLocked=false;
          logln("DRIVE: Closing b/c temp too low");
        }
      }
      if (temperature>TEMP_OPEN_ABOVE){
        if (!tempAboveLocked){
          newState=WINDOW_OPEN;
          tempBelowLocked=false;
          tempAboveLocked=true;
          logln("DRIVE: Opening b/c temp too high");
        }
      }
      if (humidity<HUM_HYSTERESIS)humidityLocked=false;
      if (humidity>HUM_OPEN_ABOVE){
        if (!humidityLocked){
          newState=WINDOW_OPEN;
          humidityLocked=true;
          logln("DRIVE: Opening b/c humidity too high");
        }
      }
    }
  
    setWindowState(newState,false);
  }else{
    if (!nightClosingDone){
      nightClosingDone=true;
      logln("DRIVE: Forcibly closing b/c it's nighttimme");
      setWindowState(WINDOW_CLOSED,true);
    }
    
    rainClosureLocked=false;
    tempBelowLocked=false;
    tempAboveLocked=false;
    humidityLocked=false;
  }
}

#endif
