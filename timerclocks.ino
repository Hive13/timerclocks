/*

 Udp NTP Client

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 created 4 Sep 2010
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe
 updated for the ESP8266 12 Apr 2015 
 by Ivan Grokhotkov
 modified 11 Aug 2016
 by Jon Neal
 Added time keeping libraries

 This code is in the public domain.

 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <Time.h> // https://github.com/PaulStoffregen/Time
#include <Timezone.h> // https://github.com/JChristensen/Timezone

#include <Adafruit_NeoPixel.h>

void ClearStrip();
void DisplayDigit(long number, uint32_t color, unsigned char show_dot);

// Which pin on the ESP is connected to the NeoPixels?
#define PIN            D1

// How many NeoPixels are attached to the ESP?
#define NUMPIXELS      246
#define NUMDISPLAYS    5
#define MAXDISPLAYVAL  99999
#define NUMSEGMENTS    7
#define LEDSPERSEGMENT 7
#define LEDSPERDIGIT   ( NUMSEGMENTS*LEDSPERSEGMENT )

char ssid[] = "hive13int";  //  your network SSID (name)
char pass[] = "hive13int";       // your network password


unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

time_t eastern, utc;
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //UTC - 5 hours
Timezone usEastern(usEDT, usEST);

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

byte displayDigits[] = {
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
};

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  
  pixels.setBrightness(10);
  pixels.begin(); // This initializes the NeoPixel library.
  DisplayDigit(pow(10, NUMDISPLAYS) - 1, pixels.Color(255,0,0), 0);


  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  setSyncProvider(ntpUpdateTime);
  setSyncInterval(30);
}

void loop()
{
  unsigned short the_time;
  utc = now();    //current time from the Time Library
  eastern = usEastern.toLocal(utc);
  // print the date
    Serial.print(year());
    Serial.print("/");
    Serial.print(month());
    Serial.print("/");
    Serial.print(day(eastern));
    // print the hour, minute and second:
    Serial.print(" ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print(hour(eastern)); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if(minute() < 10)
      Serial.print("0");
    Serial.print(minute()); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if(second() < 10)
      Serial.print("0");
    Serial.println(second()); // print the second

  the_time = second() + 100 * minute();

  DisplayDigit(the_time, pixels.Color(255,0,0), 0);


  // wait before asking for the time again
  delay(500);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

time_t ntpUpdateTime()
{
  time_t ntpTime = 0;

  Serial.println("NTP update");
  
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP); 

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  
  int cb = udp.parsePacket();
  if (!cb)
    return ntpTime;
  
  // We've received a packet, read the data from it
  udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

  //the timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, esxtract the two words:

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;


  // now convert NTP time into unix epoch:
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  ntpTime = secsSince1900 - seventyYears;
  
  // return Unix time:
  return ntpTime;
}

void DisplayNumber(long number, uint32_t color) {
  
}

void DisplayDigit(long number, uint32_t color, unsigned char show_dot) {
  long oldval;
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
  
  if (show_dot)
    pixels.setPixelColor(led++, color);

  pixels.show(); // This sends the updated pixel color to the hardware.
}

void ClearStrip() {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
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

  pixels.show(); // This sends the updated pixel color to the hardware.
}
