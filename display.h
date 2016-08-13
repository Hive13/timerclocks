#ifndef __DISPLAY_H
#define __DISPLAY_H

// Which pin on the ESP is connected to the NeoPixels?
#define PIN            D1

// How many NeoPixels are attached to the ESP?
#define NUMDISPLAYS    5
#define MAXDISPLAYVAL  99999
#define NUMSEGMENTS    7
#define LEDSPERSEGMENT 7
#define LEDSPERDIGIT   (NUMSEGMENTS * LEDSPERSEGMENT)
#define NUMPIXELS      (NUMDISPLAYS * LEDSPERDIGIT + 1)

#ifdef __cplusplus
extern "C" {
#endif

void DisplayStart(void);
void DisplaySubliminalMessage(uint32_t color);
void DisplayDigit(uint8_t number, uint8_t place, uint32_t color);
void DisplayNumber(uint32_t number, uint32_t color, unsigned char show_dot);
void ClearStrip();
void ShowPixels();
void DisplayDashes(uint32_t color);
uint32_t Color(uint8_t r, uint8_t g, uint8_t b);

static Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_H */
