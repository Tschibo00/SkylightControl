#ifndef CONFIG_H
#define CONFIG_H

#include "Preferences.h"

/*
 * Control parameters (default values are used, if not stored in flash (see readConfig)
 */
int currentWindowState = WINDOW_CLOSED;
long POLL_CYCLE_MS;             // cycle time at which conditions are evaluated. Must be longer than the driving time of the skylight!!!
float RAIN_PER_SIGNAL;          // l/m2 per rain sensor signal
float TEMP_CLOSE_BELOW;         // close the skylight below this temperature
float TEMP_OPEN_ABOVE;          // open the skylight above this temperature
float HUM_OPEN_ABOVE;           // open the skylight above this humidity
float HUM_HYSTERESIS;           // humidity must fall below this to re-enable humidity opening
long RAIN_LOCK_MS;              // time after rain detection in which convenience opening is disabled
long RAIN_THRESHOLD;            // threshold at which the rain counter at least has to change in the last cycle to trigger rain detection
long RAIN_PERIOD;               // period in which rain amount is accumulated (used for pushing to thinger.io)
Preferences prefs;

/*
 * Reads config values & window state from preferences
 */
void readConfig(){
  POLL_CYCLE_MS=prefs.getLong("POLL_CYCLE_MS",60000);
  RAIN_PER_SIGNAL=prefs.getFloat("RAIN_PER_SIGNAL",0.01f);
  TEMP_CLOSE_BELOW=prefs.getFloat("TEMP_CLOSE_BLW",25.f);
  TEMP_OPEN_ABOVE=prefs.getFloat("TEMP_OPEN_ABOVE",35.f);
  HUM_OPEN_ABOVE=prefs.getFloat("HUM_OPEN_ABOVE",70.f);
  HUM_HYSTERESIS=prefs.getFloat("HUM_HYSTERESIS",50.f);
  RAIN_LOCK_MS=prefs.getLong("RAIN_LOCK_MS",600000);
  RAIN_THRESHOLD=prefs.getLong("RAIN_THRESHOLD",1);
  RAIN_PERIOD=prefs.getLong("RAIN_PERIOD",3600000);
  currentWindowState=prefs.getInt("WindowState",WINDOW_CLOSED);
}

/*
 * Writes config values to preferences
 */
void writeConfig(){
  prefs.putLong("POLL_CYCLE_MS",POLL_CYCLE_MS);
  prefs.putFloat("RAIN_PER_SIGNAL",RAIN_PER_SIGNAL);
  prefs.putFloat("TEMP_CLOSE_BLW",TEMP_CLOSE_BELOW);
  prefs.putFloat("TEMP_OPEN_ABOVE",TEMP_OPEN_ABOVE);
  prefs.putFloat("HUM_OPEN_ABOVE",HUM_OPEN_ABOVE);
  prefs.putFloat("HUM_HYSTERESIS",HUM_HYSTERESIS);
  prefs.putLong("RAIN_LOCK_MS",RAIN_LOCK_MS);
  prefs.putLong("RAIN_THRESHOLD",RAIN_THRESHOLD);
  prefs.putLong("RAIN_PERIOD",RAIN_PERIOD);
}

/*
 * Writes window state to preferences
 */
void writeWindowState(){
  prefs.putInt("WindowState",currentWindowState);
}

/*
 * called in setup function
 */
void setupConfig(){
  prefs.begin("config",false);
  readConfig();
}

#endif
