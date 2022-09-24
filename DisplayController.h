#ifndef DISPLAY_CONTROLLER
#define DISPLAY_CONTROLLER

#include <Arduino.h>

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
//FASTLED_USING_NAMESPACE
#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

void initDisplayController();
void displayShow();
void clear(CRGB color,uint8_t buffer);
void setBrightness(int b);
void set(int x, int y, CRGB color,uint8_t buffer);
void showDigit37(uint8_t num, CRGB c, uint8_t x,uint8_t buffer,uint8_t font);
void showDigit37_font1(uint8_t num, CRGB c, uint8_t x,uint8_t buffer);
void showDigit37_font2(uint8_t num, CRGB c, uint8_t x,uint8_t buffer);
void showDigit37_font3(uint8_t num, CRGB c, uint8_t x,uint8_t buffer);
void showTime(tm t,uint8_t font);

void calcEffect();

#endif
