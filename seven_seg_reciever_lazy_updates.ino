#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN            5

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      246
#define NUMDISPLAYS    5
#define MAXDISPLAYVAL  99999
#define NUMSEGMENTS    7
#define LEDSPERSEGMENT 7
#define LEDSPERDIGIT   49

/*
For more details see: http://projectsfromtech.blogspot.com/

Connect the Rx pin of this board to the Tx of the board running Serial_Comm_Tx.ino
Connect the Grounds of the two boards
Open Serial Monitor

Receives integer value and prints it to the serial monitor
*/
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long previousUpdateMillis = 0;
unsigned long startMillis = 0;

char state = '-'; 
/*
    b = race begin. Start counting.
    s = standby. blank display.
    f = finished. display win time. 
    - = dq'd. display red hyphens
*/

long val;
long oldval;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

uint32_t currentColor = pixels.Color(255, 255, 255);

byte displayDigits[] = {
  191, /* 0 [10111111]  Painting numbers, with numbers.  */
  6,   /* 1 [00000110]                                   */ 
  91,  /* 2 [01011011]    1                              */
  79,  /* 3 [01001111]  6   2                            */ 
  102, /* 4 [01100110]    7                              */  
  109, /* 5 [01101101]  5   3                            */ 
  253, /* 6 [11111101]    4                              */ 
  7,   /* 7 [00000111]                                   */ 
  255, /* 8 [11111111]                                   */ 
  239  /* 9 [11101111]                                   */ 
};

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(10);
  pixels.setBrightness(255);
  pixels.begin(); // This initializes the NeoPixel library.

}

void loop()
{
  long incoming = Serial.available();
  
  if (incoming > 0) {
    val = Serial.parseInt();  //Reads integers as integer rather than ASCII. Anything else returns 0
  } 
  
 //* Set State 
  if(val > 0){
    state = 'f'; // Race finished. Update with real value. 
  } else if(val == -1) {
    state = 'b'; // Race begin. Set to zero and start counting. 
    startMillis = millis();
    val = -2;
  } else if(val == -22) {
    state = 's'; // Standby; blank display
  } else if(val == -33) {
    state = '-'; // Disqualified; display hyphens
  }

  /* Process State */
  currentMillis = millis();

  if(state == 'b') {
    DisplayDigit(currentMillis - startMillis, pixels.Color(0,255,0));
  } else if (state == 'f' && val > 0) {
    DisplayDigit(val, pixels.Color(0,255,0));    
  } else if (state == 's') {
    ClearStrip();
  } else if (state == '-') {
    DisplayDashes(pixels.Color(255,0,0));
  }
  
/*
  Serial.print("currentMillis: ");
  Serial.print(currentMillis);
  Serial.print("\n");
  Serial.print("previousMillis: ");
  Serial.println(previousMillis);
  Serial.print("\n");
//*/
}

 
void DisplayDigit(long number, uint32_t color) {
  if(currentMillis - previousMillis < 91) {
    return;
  }
  previousMillis = currentMillis;

  /* int number = a number from 0-99999 */
  if(number < 0 || number > MAXDISPLAYVAL) {
    return;
  }
  
  if(number == oldval) {
    return;  
  }
  oldval = number;
  
  int led = 0;
  int digits[5]; 

  pixels.clear(); 
  long divisor;
  long modulo = MAXDISPLAYVAL + 1;
  
  for(int disp = 0; disp < NUMDISPLAYS; disp++){
    divisor = modulo / 10;
    digits[disp] = (number % modulo) / divisor; 
    /* Guard against insgnificant zero to left of decimal point. */
    if((disp > 1) || (number >= divisor)) {
      for(int seg=0; seg < NUMSEGMENTS; seg++) {
        if((displayDigits[digits[disp]] & (1 << seg))) {
          for(int segLed=0; segLed < LEDSPERSEGMENT; segLed++) {
            pixels.setPixelColor(led++, color);
          }
        }
        else
          led += LEDSPERSEGMENT;
      }
    } else
      led += LEDSPERDIGIT;
    modulo = divisor;
  }
  pixels.setPixelColor(led++, color);

  pixels.show(); // This sends the updated pixel color to the hardware.
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

  pixels.show(); // This sends the updated pixel color to the hardware.
}

void ClearStrip() {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
  pixels.show();
}
