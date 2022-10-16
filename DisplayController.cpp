#include "DisplayController.h"

#define DATA_PIN 13
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 128
#define FRAMES_PER_SECOND 30
#define LIGHT_CYCLE 50

CRGB leds[NUM_LEDS * 2];
CRGB copyBuffer[NUM_LEDS];

CRGB dotColor = CRGB(128, 128, 128);
CHSV clockColor[2];
uint8_t curEffect = 0;

uint8_t levels8[8] = { 0, 1, 2, 7, 15, 40, 100, 255 };
uint8_t levels4[4] = { 0, 4, 80, 255 };
uint8_t curBuffer = 0;
uint8_t fader = 0;  // 0=no fading, 1..39=fading
uint8_t lastSec = 0;

uint8_t shuffle[32];

long lastTimeUpdate = 0L;
uint8_t dotCount = 0;

void initDisplayController() {
  for (int i = 0; i < NUM_LEDS * 2; i++) {
    leds[i] = CRGB::Black;
  }
  clockColor[0] = CHSV(0, 255, 255);
  clockColor[1] = CHSV(128, 255, 255);

  int i = 0;
  uint8_t r;
  uint8_t o;
  for (uint8_t y = 0; y < 8; y += 2)
    for (uint8_t x = 0; x < 16; x += 2)
      shuffle[i++] = x * 8 + y;
  for (i = 0; i < 1000; i++) {
    r = random(32);
    o = shuffle[i % 32];
    shuffle[i % 32] = shuffle[r];
    shuffle[r] = o;
  }

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(copyBuffer, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(1);
}

void displayShow() {

  if (fader > 0)
    calcEffect();
  else
    for (uint8_t y = 0; y < 8; y++)
      for (uint8_t x = 0; x < 16; x++)
        copyBuffer[x * 8 + 7 - y] = leds[x + y * 16 + curBuffer * NUM_LEDS];
  FastLED.show();
}

void setBrightness(int b) {
  int bn = b;
  if (bn < 0) bn = 0;
  if (bn > 255) bn = 255;
  bn+=getBrightnessCorrection();
  if (bn > 255) bn = 255;
  FastLED.setBrightness(bn);
}

int getBrightnessCorrection() {
  uint8_t maxR = 0, maxG = 0, maxB = 0;
  CRGB c;
  for (uint8_t y = 0; y < 8; y++)
    for (uint8_t x = 0; x < 16; x++)    {
      c=copyBuffer[x * 8 + 7 - y];
      if (c.r>maxR)maxR=c.r;
      if (c.g>maxR)maxG=c.g;
      if (c.b>maxR)maxB=c.b;
    }
  return (255-c.getLuma())/32;
}

void clearCopyBuffer() {
  for (uint8_t i = 0; i < NUM_LEDS; i++)
    copyBuffer[i] = CRGB::Black;
}

void clear(CRGB color, uint8_t buffer) {
  for (uint8_t i = 0; i < NUM_LEDS; i++)
    leds[i + buffer * NUM_LEDS] = color;
}

void set(int x, int y, CRGB color, uint8_t buffer) {
  if (x >= 0 && x < 16 && y >= 0 && y < 8) leds[y * 16 + x + buffer * NUM_LEDS] = color;
}

void showDigit37(uint8_t num, CRGB c, uint8_t x, uint8_t buffer, uint8_t font) {
  switch (font) {
    case 1: showDigit37_font1(num, c, x, buffer); break;
    case 2: showDigit37_font2(num, c, x, buffer); break;
    case 3: showDigit37_font3(num, c, x, buffer); break;
  }
}

// serif font
void showDigit37_font1(uint8_t num, CRGB c, uint8_t x, uint8_t buffer) {
  switch (num) {
    case 0:
      set(x + 1, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 1, 7, c, buffer);
      break;
    case 1:
      set(x + 1, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 1, 2, c, buffer);
      set(x + 1, 3, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 1, 5, c, buffer);
      set(x + 1, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 2:
      set(x + 1, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 3:
      set(x + 1, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 1, 7, c, buffer);
      break;
    case 4:
      set(x, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 5:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      break;
    case 6:
      set(x + 1, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 1, 7, c, buffer);
      break;
    case 7:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 1, 5, c, buffer);
      set(x + 1, 6, c, buffer);
      set(x + 1, 7, c, buffer);
      break;
    case 8:
      set(x + 1, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 1, 7, c, buffer);
      break;
    case 9:
      set(x + 1, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 1, 7, c, buffer);
      break;
  }
}

// Amiga intro font
void showDigit37_font2(uint8_t num, CRGB c, uint8_t x, uint8_t buffer) {
  switch (num) {
    case 0:
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 1:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 1, 2, c, buffer);
      set(x + 1, 3, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 1, 5, c, buffer);
      set(x + 1, 6, c, buffer);
      set(x + 1, 7, c, buffer);
      break;
    case 2:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 3:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 4:
      set(x, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 5:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      break;
    case 6:
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 7:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 8:
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 9:
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
  }
}

// 7-segment font
void showDigit37_font3(uint8_t num, CRGB c, uint8_t x, uint8_t buffer) {
  switch (num) {
    case 0:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 1:
      set(x + 2, 1, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 2:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 3:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 4:
      set(x, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 5:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 6:
      set(x, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 7:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 8:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x, 5, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x, 6, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x, 7, c, buffer);
      set(x + 1, 7, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
    case 9:
      set(x, 1, c, buffer);
      set(x + 1, 1, c, buffer);
      set(x + 2, 1, c, buffer);
      set(x, 2, c, buffer);
      set(x + 2, 2, c, buffer);
      set(x, 3, c, buffer);
      set(x + 2, 3, c, buffer);
      set(x, 4, c, buffer);
      set(x + 1, 4, c, buffer);
      set(x + 2, 4, c, buffer);
      set(x + 2, 5, c, buffer);
      set(x + 2, 6, c, buffer);
      set(x + 2, 7, c, buffer);
      break;
  }
}

float htr(int i) {
  i = i % 256;
  if (i <= 0) return 0.f;
  if (i >= 172) return 0.f;
  if ((i >= 43) && (i <= 128)) return 1.f;
  if (i < 43) return ((float)i) / 43.f;
  if (i >= 129) return ((float)(172 - i)) / 43.f;
}

void hsvToRgb(CHSV cIn, CRGB* cOut) {
  float w = 255 - cIn.s;
  float r = htr(cIn.h + 86) * 255.f + w;
  float g = htr(cIn.h) * 255.f + w;
  float b = htr(cIn.h - 86) * 255.f + w;
  if (r < 0.f) r = 0.f;
  if (g < 0.f) g = 0.f;
  if (b < 0.f) b = 0.f;
  if (r > 255.f) r = 255.f;
  if (g > 255.f) g = 255.f;
  if (b > 255.f) b = 255.f;
  cOut->r = r;
  cOut->g = g;
  cOut->b = b;
}

int min(int a, int b) {
  if (a < b) return a;
  else return b;
}

void showTime(tm t, uint8_t font) {
  if (millis() < lastTimeUpdate + LIGHT_CYCLE) return;
  lastTimeUpdate = millis();

  dotCount = (dotCount + 1) % 10;

  if ((fader == 0) && (dotCount != 0)) return;

  CRGB clockColorRGB;
  uint8_t m, h;
  for (uint8_t buf = 0; buf < 2; buf++) {
    m = t.tm_min;
    h = t.tm_hour;
    if (fader == 0) {
      hsvToRgb(clockColor[buf], &clockColorRGB);
      if ((clockColorRGB.r < 128) && (clockColorRGB.g < 128) && (clockColorRGB.b < 128)) {  //correct colors, that may be reduced to black at minimum brightness (2)
        if (clockColorRGB.r > 120) clockColorRGB.r = 128;
        if (clockColorRGB.g > 120) clockColorRGB.g = 128;
        if (clockColorRGB.b > 120) clockColorRGB.b = 128;
      }

      clear(CRGB::Black, buf);
      if ((buf != curBuffer) && (t.tm_sec == 59)) {
        if (m == 59) {
          m = 0;
          h++;
        } else {
          m++;
        }
      }
      if (h > 9)
        showDigit37(h / 10, clockColorRGB, 0, buf, font);
      showDigit37(h % 10, clockColorRGB, 4, buf, font);
      showDigit37(m / 10, clockColorRGB, 9, buf, font);
      showDigit37(m % 10, clockColorRGB, 13, buf, font);
    }
    if ((millis() / 500) % 2 == 0) {
      set(7, 3, dotColor, buf);
      set(8, 3, dotColor, buf);
      set(7, 5, dotColor, buf);
      set(8, 5, dotColor, buf);
    } else {
      set(7, 3, CRGB::Black, buf);
      set(8, 3, CRGB::Black, buf);
      set(7, 5, CRGB::Black, buf);
      set(8, 5, CRGB::Black, buf);
    }
  }

  // every 5 minutes fade between the two buffers
  // fading starts 1 sec before minutes flip and stops 1 sec afterwars
  // fader increases with every frame, 20fps
  if (fader > 0)
    fader = (fader + 1) % 40;
  if ((t.tm_min % 5 == 4) && (t.tm_sec == 59) && (lastSec == 58)) fader = 1;

  if (fader == 0) {
    curBuffer = (t.tm_min / 5) % 2;                                                                                     // pause switching buffers until fading is done
    clockColor[!curBuffer] = CHSV((clockColor[curBuffer].h + random(98, 198)) % 256, min(255, random(170, 300)), 255);  // move the hue more or less on the opposite site with a sight offset to advance enough
    curEffect = random(10);
  }

  displayShow();

  lastSec = t.tm_sec;
}

int clipTo15(int i) {
  if (i < 0) return 0;
  if (i > 15) return 15;
  return i;
}

CRGB flip(int i) {
  if (curBuffer == 0) return leds[i];
  return leds[(i % NUM_LEDS) + (1 - (i / NUM_LEDS)) * NUM_LEDS];
}

// ============= effects section used to blend between clock colors
void calcEffect() {
  int8_t f = clipTo15((fader - 8) / 2);

  switch (curEffect) {
    case 0:
      for (uint8_t y = 0; y < 8; y++)
        for (uint8_t x = 0; x < 16; x++)
          copyBuffer[x * 8 + 7 - y] = flip(x + y * 16 + (f > x) * NUM_LEDS);
      break;
    case 1:
      for (uint8_t y = 0; y < 8; y++)
        for (uint8_t x = 0; x < 16; x++)
          copyBuffer[x * 8 + 7 - y] = flip(x + y * 16 + (fader - 16 > y) * NUM_LEDS);
      break;
    case 2:
      for (uint8_t y = 0; y < 8; y++)
        for (uint8_t x = 0; x < 16; x++)
          if (x > 15 - f)
            copyBuffer[x * 8 + 7 - y] = flip(x - (16 - f) + y * 16 + NUM_LEDS);
          else
            copyBuffer[x * 8 + 7 - y] = flip(x + f + y * 16);
      break;
    case 3:
      for (uint8_t y = 0; y < 8; y++)
        for (uint8_t x = 0; x < 16; x++)
          if (fader < 20)
            copyBuffer[x * 8 + 7 - y] = blend(flip(x + y * 16), CRGB::DarkRed, fader * 12);
          else
            copyBuffer[x * 8 + 7 - y] = blend(flip(x + y * 16 + NUM_LEDS), CRGB::DarkRed, (39 - fader) * 12);
      break;
    case 4:
      for (uint8_t y = 0; y < 8; y++)
        for (uint8_t x = 0; x < 16; x++)
          copyBuffer[x * 8 + 7 - y] = blend(flip(x + y * 16), flip(x + y * 16 + NUM_LEDS), fader * 6);
      break;
    case 5:
      clearCopyBuffer();
      for (uint8_t y = 0; y < 8; y++)
        if (f < 8)
          for (uint8_t x = 0; x < 16 - f * 2; x++)
            copyBuffer[clipTo15(x + f) * 8 + 7 - y] = flip(x + y * 16);
        else
          for (uint8_t x = 0; x < f * 2 - 14; x++)
            copyBuffer[clipTo15(x + 15 - f) * 8 + 7 - y] = flip(x + y * 16 + NUM_LEDS);
      break;
    case 6:
      for (uint8_t y = 0; y < 8; y++)
        for (uint8_t x = 0; x < 16; x++)
          if (fader < 20)
            copyBuffer[x * 8 + 7 - y] = blend(flip(x + y * 16), CRGB::Black, fader * 12);
          else
            copyBuffer[x * 8 + 7 - y] = blend(flip(x + y * 16 + NUM_LEDS), CRGB::Black, (39 - fader) * 12);
      break;
    case 7:
      for (uint8_t y = 0; y < 8; y++)
        for (uint8_t x = 0; x < 16; x++)
          if (fader < 20)
            copyBuffer[x * 8 + 7 - y] = blend(flip(x + y * 16), CRGB::DarkGreen, fader * 12);
          else
            copyBuffer[x * 8 + 7 - y] = blend(flip(x + y * 16 + NUM_LEDS), CRGB::DarkGreen, (39 - fader) * 12);
      break;
    case 8:
      for (uint8_t y = 0; y < 8; y++)
        for (uint8_t x = 0; x < 16; x++)
          if (fader < 20)
            copyBuffer[x * 8 + 7 - y] = blend(flip(x + y * 16), CRGB::DarkBlue, fader * 12);
          else
            copyBuffer[x * 8 + 7 - y] = blend(flip(x + y * 16 + NUM_LEDS), CRGB::DarkBlue, (39 - fader) * 12);
      break;
    case 9:
      for (uint8_t y = 0; y < 8; y++)
        for (uint8_t x = 0; x < 16; x++)
          copyBuffer[x * 8 + 7 - y] = flip(x + y * 16);
      if (fader >= 8)
        for (uint8_t i = 0; i <= fader - 8; i++) {
          copyBuffer[shuffle[i]] = flip(shuffle[i] / 8 + (7 - shuffle[i] % 8) * 16 + NUM_LEDS);
          copyBuffer[shuffle[i] + 8] = flip(shuffle[i] / 8 + (7 - shuffle[i] % 8) * 16 + 1 + NUM_LEDS);
          copyBuffer[shuffle[i] + 1] = flip(shuffle[i] / 8 + (7 - shuffle[i] % 8) * 16 - 16 + NUM_LEDS);
          copyBuffer[shuffle[i] + 9] = flip(shuffle[i] / 8 + (7 - shuffle[i] % 8) * 16 - 15 + NUM_LEDS);
        }
      break;
    default:
      Serial.printf("Unknown effect %d\n", curEffect);
      break;
  }
}