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
long DEBOUNCE_MS;               // Debounce period
long LIGHT_LOWEST;              // lowest possible light sensor reading
long LIGHT_HIGHEST;             // highest possible light sensor reading
float LIGHT_EXPONENT;           // exponent to be used to light calculation (1=linear, >1 exponential, <1 logarithmic)
long LUM_LOWEST;                // lowest luminance setting for clock (at minimal light level), range 0-255
long LUM_HIGHEST;               // highest luminance setting for clock (at maximum light level), range 0-255
float TEMP_LUM_CORRECTION;      // degrees to be substracted at maximum light level, linearly decreased with decreasing light level
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
  DEBOUNCE_MS=prefs.getLong("DEBOUNCE_MS",90);
  LIGHT_LOWEST=prefs.getLong("LIGHT_LOWEST",0);
  LIGHT_HIGHEST=prefs.getLong("LIGHT_HIGHEST",2048);
  LIGHT_EXPONENT=prefs.getFloat("LIGHT_EXP",3.f);
  LUM_LOWEST=prefs.getLong("LUM_LOWEST",1);
  LUM_HIGHEST=prefs.getLong("LUM_HIGHEST",255);
  TEMP_LUM_CORRECTION=prefs.getFloat("TEMP_LUM",5.f);
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
  prefs.putLong("DEBOUNCE_MS",DEBOUNCE_MS);
  prefs.putLong("LIGHT_LOWEST",LIGHT_LOWEST);
  prefs.putLong("LIGHT_HIGHEST",LIGHT_HIGHEST);
  prefs.putFloat("LIGHT_EXP",LIGHT_EXPONENT);
  prefs.putLong("LUM_LOWEST",LUM_LOWEST);
  prefs.putLong("LUM_HIGHEST",LUM_HIGHEST);
  prefs.putFloat("TEMP_LUM",TEMP_LUM_CORRECTION);
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
