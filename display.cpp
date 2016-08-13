#include <Adafruit_NeoPixel.h>

#include "display.h"

static byte displayDigits[] = {
  191, /* 0 [00111111]  Painting numbers, with numbers.  */
  6,   /* 1 [00000110]                                   */
  91,  /* 2 [01011011]    1                              */
  79,  /* 3 [01001111]  6   2                            */
  102, /* 4 [01100110]    7                              */
  109, /* 5 [01101101]  5   3                            */
  253, /* 6 [01111101]    4                              */
  7,   /* 7 [00000111]                                   */ 
  255, /* 8 [01111111]                                   */
  239, /* 9 [01101111]                                   */
  119, /* A [01110111]                                   */
  140, /* b [01111100]                                   */
  88,  /* c [01011000]                                   */
  94,  /* d [01011110]                                   */
  121, /* E [01111001]                                   */
  113, /* F [01110001]                                   */
  84,  /* n [01010100] 16                                */
  119, /* A [01110111]                                   */
  121, /* E [01111001]  E is double defined              */
  56,  /* L [00111000]                                   */
  57,  /* C [00111001]                                   */
};


void DisplayStart(void) {
	pixels.setBrightness(10);
	pixels.begin();
}

uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
	return pixels.Color(r, g, b);
}


// place starts from zero for the farthest right digit
void DisplaySubliminalMessage(uint32_t color) {
  pixels.clear();
  for( int currentLetter = 0; currentLetter < 6; currentLetter++) {
    for( int seg=0; seg < NUMSEGMENTS; seg++ ) {
      if( displayDigits[currentLetter + 16] & (1 << seg) ) {
        for( int segLed=0; segLed < LEDSPERSEGMENT; segLed++ ) {
          pixels.setPixelColor( (NUMPIXELS - LEDSPERDIGIT) - LEDSPERDIGIT*currentLetter + LEDSPERSEGMENT*seg + segLed - 1, color );
        }
      }
    }
  }
  
  //pixels.show(); // This sends the updated pixel color to the hardware.
}

// place starts from zero for the farthest right digit
void DisplayDigit(uint8_t number, uint8_t place, uint32_t color) {
  for( int seg=0; seg < NUMSEGMENTS; seg++ ) {
    if( displayDigits[number] & (1 << seg) ) {
      for( int segLed=0; segLed < LEDSPERSEGMENT; segLed++ ) {
        pixels.setPixelColor( (NUMPIXELS - LEDSPERDIGIT) - LEDSPERDIGIT*place + LEDSPERSEGMENT*seg + segLed - 1, color );
      }
    }
  }
  
  //pixels.show(); // This sends the updated pixel color to the hardware.
}


void DisplayHour(uint8_t hour, uint32_t color) {
  DisplayDigit(hour%10, 3, color);
  if(hour/10 > 0)
    DisplayDigit(hour/10, 4, color);
}

void DisplayMinutes(uint8_t minutes, uint32_t color) {
  DisplayDigit(minutes%10, 0, color);
  DisplayDigit(minutes/10, 1, color);
}


void DisplayNumber(uint32_t number, uint32_t color, unsigned char show_dot) {
  uint32_t currentNumber = 0;
  
  /* int number = a number from 0-99999 */
  if(number < 0 || number > MAXDISPLAYVAL) {
    return;
  }
  
  pixels.clear(); 

  currentNumber = number % 10;
  for(uint32_t currentPlace = 0; number > 0; number /= 10, currentNumber = number % 10, ++currentPlace)
  {
    DisplayDigit( currentNumber, currentPlace, pixels.Color(255, 0, 0) );
  }

}

void ClearStrip() {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
  //pixels.show();
}

void ShowPixels() {
  pixels.show();
}

void DisplayDashes(uint32_t color) {
  pixels.clear(); 
  int led = 0;
  for(int disp = 0; disp < NUMDISPLAYS; disp++){
    for(int seg=0; seg < NUMSEGMENTS; seg++) {
      if(6 == seg) {
        for(int segLed=0; segLed < LEDSPERSEGMENT; segLed++) {
          pixels.setPixelColor(led++, color);
        }
      }
      else
      {
        led += NUMSEGMENTS;
      }
    }
  }

  //pixels.show(); // This sends the updated pixel color to the hardware.
}
