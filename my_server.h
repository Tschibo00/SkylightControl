#ifndef MY_SERVER_H
#define MY_SERVER_H

#include <WebServer.h>
#include <Update.h>
#include "config.h"
#include "localtime.h"
#include "my_state.h"

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

WebServer server(80);
char htmlBuf[10240];

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
 * called in setup function
 */
void setupServer(){
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
    snprintf(htmlBuf,sizeof(htmlBuf),"<html><body><form action=\"/configset\">cycle time at which conditions are evaluated. Must be longer than the driving time of the skylight!!!<br/>POLL_CYCLE_MS<input type=\"number\" min=100 name=\"POLL_CYCLE_MS\" value=\"%d\"><br/>l/m2 per rain sensor signal<br/>RAIN_PER_SIGNAL<input type=\"number\" min=0.001 step=0.001 name=\"RAIN_PER_SIGNAL\" value=\"%f\"><br/>close the skylight below this temperature<br/>TEMP_CLOSE_BELOW<input type=\"number\" step=0.1 min=10 max=40 name=\"TEMP_CLOSE_BELOW\" value=\"%.1f\"><br/>open the skylight above this temperature<br/>TEMP_OPEN_ABOVE<input type=\"number\" step=0.1 min=10 max=40 name=\"TEMP_OPEN_ABOVE\" value=\"%.1f\"><br/>open the skylight above this humidity<br/>HUM_OPEN_ABOVE<input type=\"number\" step=1 min=1 max=99 name=\"HUM_OPEN_ABOVE\" value=\"%.1f\"><br/>humidity must fall below this to re-enable humidity opening<br/>HUM_HYSTERESIS<input type=\"number\" step=1 min=1 max=99 name=\"HUM_HYSTERESIS\" value=\"%.1f\"><br/>time after rain detection in which convenience opening is disabled<br/>RAIN_LOCK_MS<input type=\"number\" min=100 name=\"RAIN_LOCK_MS\" value=\"%d\"><br/>threshold at which the rain counter at least has to change in the last cycle to trigger rain detection<br/>RAIN_THRESHOLD<input type=\"number\" min=1 name=\"RAIN_THRESHOLD\" value=\"%d\"><br/>period in which rain amount is accumulated (used for pushing to thinger.io)<br/>RAIN_PERIOD<input type=\"number\" min=100 name=\"RAIN_PERIOD\" value=\"%d\"><br/>Debounce period<br/>DEBOUNCE_MS<input type=\"number\" min=1 max=100 name=\"DEBOUNCE_MS\" value=\"%d\"><br/>lowest possible light sensor reading<br/>LIGHT_LOWEST<input type=\"number\" min=0 max=4095 name=\"LIGHT_LOWEST\" value=\"%d\"><br/>highest possible light sensor reading<br/>LIGHT_HIGHEST<input type=\"number\" min=0 max=4095 name=\"LIGHT_HIGHEST\" value=\"%d\"><br/>exponent to be used to light calculation (1=linear, >1 exponential, <1 logarithmic)<br/>LIGHT_EXPONENT<input type=\"number\" min=0.0 max=10.0 step=0.1 name=\"LIGHT_EXPONENT\" value=\"%f\"><br/>lowest luminance setting for clock (at minimal light level), range 0-255<br/>LUM_LOWEST<input type=\"number\" min=0 max=255 name=\"LUM_LOWEST\" value=\"%d\"><br/>highest luminance setting for clock (at maximum light level), range 0-255<br/>LUM_HIGHEST<input type=\"number\" min=0 max=255 name=\"LUM_HIGHEST\" value=\"%d\"><br/>degrees to be substracted at maximum light level, linearly decreased with decreasing light level<br/>TEMP_LUM_CORRECTION<input type=\"number\" min=0.0 max=20.0 step=0.1 name=\"TEMP_LUM_CORRECTION\" value=\"%f\"><br/><input type=\"submit\" value=\"Send\"></form></body></html>",POLL_CYCLE_MS,RAIN_PER_SIGNAL,TEMP_CLOSE_BELOW,TEMP_OPEN_ABOVE,HUM_OPEN_ABOVE,HUM_HYSTERESIS,RAIN_LOCK_MS,RAIN_THRESHOLD,RAIN_PERIOD,DEBOUNCE_MS,LIGHT_LOWEST,LIGHT_HIGHEST,LIGHT_EXPONENT,LUM_LOWEST,LUM_HIGHEST,TEMP_LUM_CORRECTION);
    server.send(200, "text/html",htmlBuf);
  });
  server.on("/configset", HTTP_GET, [](){
    Serial.println("got config");
      for (int i = 0; i < server.args(); i++){
    Serial.print(server.argName(i));
    Serial.print("=");
    Serial.println(server.arg(i));}
    if (server.args()) {
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
        if (server.argName(i)=="DEBOUNCE_MS") DEBOUNCE_MS=strtol(server.arg(i).c_str(),0,0);
        if (server.argName(i)=="LIGHT_LOWEST") LIGHT_LOWEST=strtol(server.arg(i).c_str(),0,0);
        if (server.argName(i)=="LIGHT_HIGHEST") LIGHT_HIGHEST=strtol(server.arg(i).c_str(),0,0);
        if (server.argName(i)=="LIGHT_EXPONENT") LIGHT_EXPONENT=strtof(server.arg(i).c_str(),0);
        if (server.argName(i)=="LUM_LOWEST") LUM_LOWEST=strtol(server.arg(i).c_str(),0,0);
        if (server.argName(i)=="LUM_HIGHEST") LUM_HIGHEST=strtol(server.arg(i).c_str(),0,0);
        if (server.argName(i)=="TEMP_LUM_CORRECTION") TEMP_LUM_CORRECTION=strtof(server.arg(i).c_str(),0);
      }
      if ((TEMP_CLOSE_BELOW>=TEMP_OPEN_ABOVE)||(HUM_HYSTERESIS>=HUM_OPEN_ABOVE)){
        readConfig();
        server.sendHeader("Connection", "close");
        server.send(400, "text/html", "invalid values");
      }else{
        writeConfig();
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", "<html><head><meta http-equiv=\"refresh\" content=\"0; url=/config\" /></head></html>");
      }
    }else{
      server.sendHeader("Connection", "close");
      server.send(400, "text/html", "empty config");
    }
  });
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      printLocalTime();
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
        printLocalTime();
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

void serverHandle(){
  server.handleClient();
}

 #endif
