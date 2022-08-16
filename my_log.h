#ifndef MY_LOG_H
#define MY_LOG_H

#include "localtime.h"

void log(char *s){
  printLocalTime();
  Serial.print(s);
}

void logln(char *s){
  printLocalTime();
  Serial.println(s);
}

#endif
