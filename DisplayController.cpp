#include "DisplayController.h"
//#include "font.h"

#define DATA_PIN    13
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    128
#define FRAMES_PER_SECOND  30
#define LIGHT_CYCLE 50

CRGB leds[NUM_LEDS*2];
CRGB copyBuffer[NUM_LEDS];

CRGB dotColor=CRGB(128,128,128);
CHSV clockColor[2];

uint8_t levels8[8]={0,1,2,7,15,40,100,255};
uint8_t levels4[4]={0,4,80,255};
uint8_t curBuffer=0;
uint8_t fader=0;      // 0=no fading, 1..39=fading
uint8_t lastSec=0;

long lastTimeUpdate=0L;

void initDisplayController(){
	for (int i = 0; i < NUM_LEDS*2; i++) {
		leds[i] = CRGB::Black;
	}
  clockColor[0]=CHSV(0,255,255);
  clockColor[1]=CHSV(128,255,255);
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(copyBuffer, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(1);
}

void displayShow(){
  for (uint8_t y=0;y<8;y++)
    for (uint8_t x=0;x<16;x++)
      copyBuffer[x*8+7-y]=leds[x+y*16+curBuffer*NUM_LEDS];
  FastLED.show();  
}

void setBrightness(int b){
  FastLED.setBrightness(b);
}

void clear(CRGB color,uint8_t buffer){
	for (uint8_t i = 0; i < NUM_LEDS; i++)
		leds[i+buffer*NUM_LEDS] = color;
}

void set(int x, int y, CRGB color,uint8_t buffer){
	if (x >= 0 && x < 16 && y >= 0 && y < 8) leds[y * 16 + x+buffer*NUM_LEDS] = color;
}

void showDigit37(uint8_t num, CRGB c, uint8_t x,uint8_t buffer){
  switch(num){
    case 0:
      set(x+1,1,c,buffer);
      set(x,2,c,buffer); set(x+2,2,c,buffer);
      set(x,3,c,buffer); set(x+2,3,c,buffer);
      set(x,4,c,buffer); set(x+2,4,c,buffer);
      set(x,5,c,buffer); set(x+2,5,c,buffer);
      set(x,6,c,buffer); set(x+2,6,c,buffer);
      set(x+1,7,c,buffer);
      break;
    case 1:
      set(x+1,1,c,buffer);
      set(x,2,c,buffer); set(x+1,2,c,buffer);
      set(x+1,3,c,buffer);
      set(x+1,4,c,buffer);
      set(x+1,5,c,buffer);
      set(x+1,6,c,buffer);
      set(x,7,c,buffer);set(x+1,7,c,buffer);set(x+2,7,c,buffer);
      break;
    case 2:
      set(x+1,1,c,buffer);
      set(x,2,c,buffer); set(x+2,2,c,buffer);
      set(x+2,3,c,buffer);
      set(x+1,4,c,buffer);
      set(x,5,c,buffer);
      set(x,6,c,buffer);
      set(x,7,c,buffer); set(x+1,7,c,buffer); set(x+2,7,c,buffer);
      break;
    case 3:
      set(x+1,1,c,buffer);
      set(x,2,c,buffer); set(x+2,2,c,buffer);
      set(x+2,3,c,buffer);
      set(x+1,4,c,buffer);
      set(x+2,5,c,buffer);
      set(x,6,c,buffer); set(x+2,6,c,buffer);
      set(x+1,7,c,buffer);
      break;
    case 4:
      set(x,1,c,buffer);
      set(x,2,c,buffer);
      set(x,3,c,buffer); set(x+2,3,c,buffer);
      set(x,4,c,buffer); set(x+1,4,c,buffer); set(x+2,4,c,buffer);
      set(x+2,5,c,buffer);
      set(x+2,6,c,buffer);
      set(x+2,7,c,buffer);
      break;
    case 5:
      set(x,1,c,buffer); set(x+1,1,c,buffer); set(x+2,1,c,buffer);
      set(x,2,c,buffer);
      set(x,3,c,buffer);
      set(x,4,c,buffer); set(x+1,4,c,buffer);
      set(x+2,5,c,buffer);
      set(x+2,6,c,buffer);
      set(x,7,c,buffer); set(x+1,7,c,buffer);
      break;
    case 6:
      set(x+1,1,c,buffer);
      set(x,2,c,buffer); set(x+2,2,c,buffer);
      set(x,3,c,buffer);
      set(x,4,c,buffer); set(x+1,4,c,buffer);
      set(x,5,c,buffer); set(x+2,5,c,buffer);
      set(x,6,c,buffer); set(x+2,6,c,buffer);
      set(x+1,7,c,buffer);
      break;
    case 7:
      set(x,1,c,buffer); set(x+1,1,c,buffer); set(x+2,1,c,buffer);
      set(x+2,2,c,buffer);
      set(x+2,3,c,buffer);
      set(x+1,4,c,buffer);
      set(x+1,5,c,buffer);
      set(x+1,6,c,buffer);
      set(x+1,7,c,buffer);
      break;
    case 8:
      set(x+1,1,c,buffer);
      set(x,2,c,buffer); set(x+2,2,c,buffer);
      set(x,3,c,buffer); set(x+2,3,c,buffer);
      set(x+1,4,c,buffer);
      set(x,5,c,buffer); set(x+2,5,c,buffer);
      set(x,6,c,buffer); set(x+2,6,c,buffer);
      set(x+1,7,c,buffer);
      break;
    case 9:
      set(x+1,1,c,buffer);
      set(x,2,c,buffer); set(x+2,2,c,buffer);
      set(x,3,c,buffer); set(x+2,3,c,buffer);
      set(x+1,4,c,buffer); set(x+2,4,c,buffer);
      set(x+2,5,c,buffer);
      set(x,6,c,buffer);set(x+2,6,c,buffer);
      set(x+1,7,c,buffer);
      break;
  }
}

void showTime(tm t){
  if (millis()<lastTimeUpdate+LIGHT_CYCLE)return;
  lastTimeUpdate=millis();

  CRGB clockColorRGB;
  for (uint8_t buf=0;buf<2;buf++){
    hsv2rgb_rainbow(clockColor[buf],clockColorRGB);
    clear(CRGB::Black,buf);
    if (t.tm_hour>9)
      showDigit37(t.tm_hour/10, clockColorRGB,0,buf);
    showDigit37(t.tm_hour%10, clockColorRGB,4,buf);
    showDigit37(t.tm_min/10, clockColorRGB,9,buf);
    showDigit37(t.tm_min%10, clockColorRGB,13,buf);
    if ((millis()/500)%2==0){
      set(7,3,dotColor,buf);
      set(8,3,dotColor,buf);
      set(7,5,dotColor,buf);
      set(8,5,dotColor,buf);
    }
  }

  // every 5 minutes fade between the two buffers
  // fading starts 1 sec before minutes flip and stops 1 sec afterwars
  // fader increases with every frame, 20fps
  if (fader>0)
    fader=(fader+1)%40;
  if ((t.tm_min%5==4)&&(t.tm_sec==59)&&(lastSec==58))fader=1;

  displayShow();
  
  if (fader==0){
    curBuffer=(t.tm_min/5)%2;// pause switching buffers until fading is done
    clockColor[!curBuffer]=CHSV((clockColor[curBuffer].h+random(88,168))%256,random(100,255),255);
  }
    
  lastSec=t.tm_sec;
}
