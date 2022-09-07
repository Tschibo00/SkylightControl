#ifndef DISPLAY_CONTROLLER
#define DISPLAY_CONTROLLER

#include <Arduino.h>

#define EFFECT_NONE 0
#define EFFECT_RAINBOW 1
#define EFFECT_FADE 2

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
//FASTLED_USING_NAMESPACE
#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

struct Point{
  int x,y;
};

void initDisplayController();
void displayShow();
void clear(CRGB color);
void copy(const uint8_t *source);
/*void copy(const char *source);
void showDigit(char row, char number);
void showNumber(int number);*/

void showNumber37(int8_t num);

void showScroller(const char *text, int scrollPos, bool smooth, CRGB color);
void showScrollerRainbow(const char *text, int scrollPos, bool smooth);
void showScroller(const char *text, int scrollPos, bool smooth, uint8_t effect, CRGB color1,CRGB color2);
void showScroller(const char *text, int scrollPos, bool smooth, bool rainbow, CRGB color);

void showCharacter(unsigned char c);
void showCharacter(unsigned char c, int8_t xOffset, CRGB color);
void showCharacter(unsigned char c, int8_t xOffset, CRGB colorTop,CRGB colorBottom);
void showCharacterRainbow(unsigned char c, int8_t xOffset);
void showCharacter(unsigned char c, int8_t xOffset, int rainbow, CRGB color);

CRGB getFullColor(uint8_t c);

void drawLine(Point p0,Point p1,CRGB color);
void drawLine(Point p,Point d, uint8_t steps, CRGB color);

/*void drawLine(int x, int y, int dx, int dy, int steps, char color);
void drawDottedLine(int x, int y, int dx, int dy, int steps, char color1, char color2);
void drawRectangle(int x, int y, int width, int height, char color);
void drawBox(int x, int y, int width, int height, char color);
void drawFade(char c0, char c1, char c2, char c3, char c4);*/
void set(int x, int y, CRGB color);
bool setTest(int x, int y, CRGB color);
void set(Point p,CRGB color);
bool setTest(Point p,CRGB color);
CRGB get(int x, int y);
CRGB get(Point p);
/*void fade(char steps);
void strike();
char* getPalette();
uint8_t* getScreen();*/

void showDigit37(uint8_t num, CRGB c, uint8_t x);
void showNumber37(uint8_t num);

void nblendU8TowardU8( uint8_t& cur, const uint8_t target, uint8_t amount);
CRGB fadeTowardColor( CRGB& cur, const CRGB& target, uint8_t amount);


#endif
