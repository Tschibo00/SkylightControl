/*
 * wire BME280 as follows:
 * Vin 3.3V
 * GND GND
 * SCL GPIO 22
 * SDA GPIO 21
 * 
 * wire relais as follows
 * open window relais - 25
 * close window relais - 26
 * 
 * wire rain sensor as follows
 * bucket contact - 18
 * 
 * For local debugging attach pots to 34+35 for temperature and humidity
 * define DEBUG_LOCAL to enable local debugging via pots
 */

/*
 * TODOs HW check
 * pulse frequency of rain sensor
 * do double signals in one direction trigger reverse driving of skylight?
 */

//#define THINGER_SERIAL_DEBUG
//#define DEBUG_LOCAL

#include <ThingerESP32.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "secrets.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include "Preferences.h"

#define WINDOW_IGNORE -1
#define WINDOW_OPEN 0
#define WINDOW_CLOSED 1

/*
 * HW parameters
 */
#define RELAIS_ACTIVE_MS 100    // switch cycle for the relais
long POLL_CYCLE_MS=10000;       // cycle time at which conditions are evaluated. Must be longer than the driving time of the skylight!!!
#define PIN_RELAIS_OPEN 25      // relais to open the skylight
#define PIN_RELAIS_CLOSE 26     // relais to close the skylight
#define PIN_RELAIS_ACTIVE 0     // 0 ACTIVE low, 1 ACTIVE high
#define PIN_RAIN 18             // bucket simulation input pin (switch to ground, pulled-up internally)
#define DEBOUNCE_MS 50          // Debounce period
float RAIN_PER_SIGNAL=0.01f;    // l/m2 per rain sensor signal

/*
 * Control parameters
 */
float TEMP_CLOSE_BELOW=25.f;    // close the skylight below this temperature
float TEMP_OPEN_ABOVE=35.f;     // open the skylight above this temperature
float HUM_OPEN_ABOVE=70.f;      // open the skylight above this humidity
float HUM_HYSTERESIS=50.f;      // humidity must fall below this to re-enable humidity opening
long RAIN_LOCK_MS=600000;       // time after rain detection in which convenience opening is disabled
long RAIN_THRESHOLD=1;          // threshold at which the rain counter at least has to change in the last cycle to trigger rain detection
long RAIN_PERIOD=3600000;       // period in which rain amount is accumulated (used for pushing to thinger.io)
Preferences prefs;
 
Adafruit_BME280 bme;            // sensor library
float temperature;              // environment readings
float pressure;
float humidity;
bool raining=false;
long rainPerHour;
bool humidityLocked=false;
bool tempAboveLocked=false;
bool tempBelowLocked=false;
bool rainClosureLocked=false;

int currentWindowState = WINDOW_CLOSED;
long oldRainBucketCount=0l;     // used to calculate if rain starts falling
long newRainBucketCount=0l;     // increased by interrupt
long unlockRainClosure=0l;      // timing variables
long rainPinDebounce=0l;
long nextSensorReadout=0l;
long nextRainAccumulation=0l;
long lastAccumulatedRainValue=0l;
float rainAmount=0.f;

ThingerESP32 thing(THINGER_USER, THINGER_ID, THINGER_TOKEN);
WebServer server(80);
char htmlBuf[10240];

/*
 * Server Index Page
 */
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

void setup() {
  Serial.begin(115200);

  pinMode(PIN_RAIN,INPUT_PULLUP);
  pinMode(PIN_RELAIS_OPEN,OUTPUT);
  pinMode(PIN_RELAIS_CLOSE,OUTPUT);
  #ifdef DEBUG_LOCAL
  pinMode(34,INPUT);
  pinMode(35,INPUT);
  #endif

  setRelaisState(WINDOW_IGNORE);
  
  WiFi.begin(WIFI_SSID,WIFI_PW);
  while (WiFi.status()!=WL_CONNECTED){delay(500);Serial.print(".");}
  Serial.println(WiFi.localIP());

  thing.add_wifi(WIFI_SSID, WIFI_PW);
  thing["temperature"] >> outputValue(temperature);
  thing["humidity"] >> outputValue(humidity);
  thing["pressure"] >> outputValue(pressure);
  thing["rain"] >> outputValue(rainAmount);

  bme.begin(0x76);

  server.on("/", HTTP_GET, [](){
    server.sendHeader("Connection","close");
    snprintf(htmlBuf,sizeof(htmlBuf),"<html><body><h1>Aktuelle Werte</h1><table><tr><td>Temperatur</td><td>%.1f &deg;C</td></tr><tr><td>Feuchtigkeit</td><td>%.1f &percnt;</td></tr><tr><td>Luftdruck</td><td>%.1f hPa</td></tr><tr></tr><tr><td>Regen?</td><td>%d</td></tr><tr><td>Regenmenge l/h</td><td>%.2f</td></tr><tr><td>Fenster</td><td>%s</td></tr></table><br/><a href=\"/config\">Konfiguration</a><br/><a href=\"serverIndex\">OTA Update</a></body></html>",temperature,humidity,pressure,raining,rainAmount,getWindowPos());
    server.send(200,"text/html",htmlBuf);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on("/config",HTTP_GET,[](){
    server.sendHeader("Connection", "close");
    snprintf(htmlBuf,sizeof(htmlBuf),"<html><body><form action=\"/configset\">cycle time at which conditions are evaluated. Must be longer than the driving time of the skylight!!!</br>POLL_CYCLE_MS<input type=\"number\" name=\"POLL_CYCLE_MS\" value=\"%d\"><br/>l/m2 per rain sensor signal</br>RAIN_PER_SIGNAL<input type=\"number\" name=\"RAIN_PER_SIGNAL\" value=\"%f\"><br/>close the skylight below this temperature<br/>TEMP_CLOSE_BELOW<input type=\"number\" name=\"TEMP_CLOSE_BELOW\" value=\"%.1f\"><br/>open the skylight above this temperature<br/>TEMP_OPEN_ABOVE<input type=\"number\" name=\"TEMP_OPEN_ABOVE\" value=\"%.1f\"><br/>open the skylight above this humidity<br/>HUM_OPEN_ABOVE<input type=\"number\" name=\"HUM_OPEN_ABOVE\" value=\"%.1f\"><br/>humidity must fall below this to re-enable humidity opening<br/>HUM_HYSTERESIS<input type=\"number\" name=\"HUM_HYSTERESIS\" value=\"%.1f\"><br/>time after rain detection in which convenience opening is disabled<br/>RAIN_LOCK_MS<input type=\"number\" name=\"RAIN_LOCK_MS\" value=\"%d\"><br/>threshold at which the rain counter at least has to change in the last cycle to trigger rain detection<br/>RAIN_THRESHOLD<input type=\"number\" name=\"RAIN_THRESHOLD\" value=\"%d\"><br/>period in which rain amount is accumulated (used for pushing to thinger.io)<br/>RAIN_PERIOD<input type=\"number\" name=\"RAIN_PERIOD\" value=\"%d\"><br/><input type=\"submit\" value=\"Send\"></form></body></html>",POLL_CYCLE_MS,RAIN_PER_SIGNAL,TEMP_CLOSE_BELOW,TEMP_OPEN_ABOVE,HUM_OPEN_ABOVE,HUM_HYSTERESIS,RAIN_LOCK_MS,RAIN_THRESHOLD,RAIN_PERIOD);
    server.send(200, "text/html",htmlBuf);
  });
  server.on("/configset", HTTP_GET, [](){
    if (server.args()) 
      for (int i = 0; i < server.args(); i++){
        if (server.argName(i)=="POLL_CYCLE_MS") POLL_CYCLE_MS=strtol(server.arg(i).c_str(),0,0);
        if (server.argName(i)=="RAIN_PER_SIGNAL") RAIN_PER_SIGNAL=strtof(server.arg(i).c_str(),0);
        if (server.argName(i)=="TEMP_CLOSE_BELOW") TEMP_CLOSE_BELOW=strtof(server.arg(i).c_str(),0);
        if (server.argName(i)=="TEMP_OPEN_ABOVE") TEMP_OPEN_ABOVE=strtof(server.arg(i).c_str(),0);
        if (server.argName(i)=="HUM_OPEN_ABOVE") HUM_OPEN_ABOVE=strtof(server.arg(i).c_str(),0);
        if (server.argName(i)=="HUM_HYSTERESIS") HUM_HYSTERESIS=strtof(server.arg(i).c_str(),0);
        if (server.argName(i)=="RAIN_LOCK_MS") RAIN_LOCK_MS=strtol(server.arg(i).c_str(),0,0);
        if (server.argName(i)=="RAIN_THRESHOLD") RAIN_THRESHOLD=strtol(server.arg(i).c_str(),0,0);
        if (server.argName(i)=="RAIN_PERIOD") RAIN_PERIOD=strtol(server.arg(i).c_str(),0,0);
      }
    writeConfig();
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", "<html><head><meta http-equiv=\"refresh\" content=\"0; url=/config\" /></head></html>");
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();

  prefs.begin("config",false);
  readConfig();
  
  attachInterrupt(PIN_RAIN, onRainTrigger, FALLING);

  Serial.println("Wintergarten Steuerung starting");
}

/*
 * Reads config values & window state from preferences
 */
void readConfig(){
  POLL_CYCLE_MS=prefs.getLong("POLL_CYCLE_MS",60000);
  RAIN_PER_SIGNAL=prefs.getFloat("RAIN_PER_SIGNAL",0.01f);
  TEMP_CLOSE_BELOW=prefs.getFloat("TEMP_CLOSE_BELOW",25.f);
  TEMP_OPEN_ABOVE=prefs.getFloat("TEMP_OPEN_ABOVE",35.f);
  HUM_OPEN_ABOVE=prefs.getFloat("HUM_OPEN_ABOVE",70.f);
  HUM_HYSTERESIS=prefs.getFloat("HUM_HYSTERESIS",50.f);
  RAIN_LOCK_MS=prefs.getLong("RAIN_LOCK_MS",600000);
  RAIN_THRESHOLD=prefs.getLong("RAIN_THRESHOLD",1);
  RAIN_PERIOD=prefs.getLong("RAIN_PERIOD",3600000);
  currentWindowState=prefs.getInt("currentWindowState",WINDOW_CLOSED);
}

/*
 * Writes config values to preferences
 */
void writeConfig(){
  prefs.putLong("POLL_CYCLE_MS",POLL_CYCLE_MS);
  prefs.putFloat("RAIN_PER_SIGNAL",RAIN_PER_SIGNAL);
  prefs.putFloat("TEMP_CLOSE_BELOW",TEMP_CLOSE_BELOW);
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
  prefs.putInt("currentWindowState",currentWindowState);
}

/*
 * returns string for window position
 */
const char* getWindowPos(){
  switch(currentWindowState){
    case WINDOW_OPEN: return "offen";
    case WINDOW_CLOSED: return "geschlossen";
  }
}

/*
 * interrupt function called when rain sensor triggers
 */
void onRainTrigger(){
  if (millis()>rainPinDebounce){
    newRainBucketCount++;
    rainPinDebounce=millis()+DEBOUNCE_MS;
  }
}

/*
 * BME280 sensor readout functions
 */
void readTemperatureSensor(){                                 // in degree C
  #ifdef DEBUG_LOCAL
  temperature=((float)analogRead(34))/200.f+20.f;
  #else
  temperature=bme.readTemperature();
  #endif
}
void readPressureSensor(){pressure=bme.readPressure()/100.f;} // in hPa
void readHumiditySensor(){                                    // in Percent
  #ifdef DEBUG_LOCAL
  humidity=((float)analogRead(35))/41.f;
  #else
  humidity=bme.readHumidity();
  #endif
}

/*
 * Signals rain if a certain threshold is passed
 */
void readRainSensor(){
  if (newRainBucketCount>=oldRainBucketCount+RAIN_THRESHOLD){
    oldRainBucketCount=newRainBucketCount;
    raining=true;

    // instantly force close if rain starts
    if (!rainClosureLocked){
      Serial.println("DRIVE: Forcibly closing b/c it's raining");
      setWindowState(WINDOW_CLOSED,true);
    }
    unlockRainClosure=millis()+RAIN_LOCK_MS;    // extend lock time if it's still raining
    rainClosureLocked=true;
  }else{
    raining=false;
  }
}

/*
 * Calculates accumulated amount of rain in given period
 */
void calculateRainAmount(){
  if (millis()>=nextRainAccumulation){
    nextRainAccumulation=millis()+RAIN_PERIOD;
    rainAmount=(newRainBucketCount-lastAccumulatedRainValue)*RAIN_PER_SIGNAL;
    lastAccumulatedRainValue=newRainBucketCount;
  }
}

/*
 * Business logic. Determines, based on environment parameters, if the skylight needs to be open or closed
 */
void evaluteWindowPosition(){
  int newState=WINDOW_IGNORE;

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
        Serial.println("DRIVE: Closing b/c temp too low");
      }
    }
    if (temperature>TEMP_OPEN_ABOVE){
      if (!tempAboveLocked){
        newState=WINDOW_OPEN;
        tempBelowLocked=false;
        tempAboveLocked=true;
        Serial.println("DRIVE: Opening b/c temp too high");
      }
    }
    if (humidity<HUM_HYSTERESIS)humidityLocked=false;
    if (humidity>HUM_OPEN_ABOVE){
      if (!humidityLocked){
        newState=WINDOW_OPEN;
        humidityLocked=true;
        Serial.println("DRIVE: Opening b/c humidity too high");
      }
    }
  }

  setWindowState(newState,false);
}

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
 * Drives the relais based on the required state
 */
void setOutput(int relaisState){
  setRelaisState(relaisState);
  delay(RELAIS_ACTIVE_MS);
  setRelaisState(WINDOW_IGNORE);
}

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

void loop() {
  if (millis()>=nextSensorReadout){
    nextSensorReadout=millis()+POLL_CYCLE_MS;
  
    readRainSensor();
    readTemperatureSensor();
    readPressureSensor();
    readHumiditySensor();
    calculateRainAmount();
  
    Serial.print("Rain ");
    Serial.print(raining);
    Serial.print(" (");
    Serial.print(newRainBucketCount);
    Serial.print(") ");
    Serial.print(temperature);
    Serial.print("C ");
    Serial.print(humidity);
    Serial.print("% ");
    Serial.print(pressure);
    Serial.println("hPa");
  
    evaluteWindowPosition();
  }

  thing.handle();
  server.handleClient();
}
