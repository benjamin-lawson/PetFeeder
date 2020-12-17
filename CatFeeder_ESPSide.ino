/* 
  ----------------------------------------
  |                                      |
  |              Libraries               |
  |                                      |
  ----------------------------------------
*/
#include <WiFi.h>

// NTC Required Libraries
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi Manager Required Libraries
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>

// LED Ring Required Libraries
#include <FastLED.h>

// Screen Required Libraries
#include <U8glib.h>
/* End Libraries */

/* 
  ----------------------------------------
  |                                      |
  |        Contstant Definitions         |
  |                                      |
  ----------------------------------------
*/
#define DEBUG true
#define UPDATE_TIME_MS 15000
#define NUM_LEDS 24
const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};
/* End Constant Defintions */

/* 
  ----------------------------------------
  |                                      |
  |           Pin Definitions            |
  |                                      |
  ----------------------------------------
*/
#define LCD_RST 5
#define LCD_CS 10
#define LCD_DC 8
#define LCD_MOSI 36
#define LCD_SCL 39

#define LED_RING 12
/* End Pin Definitions */

/* 
  ----------------------------------------
  |                                      |
  |         Variable Definitions         |
  |                                      |
  ----------------------------------------
*/
WiFiManager wifiManager;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
U8GLIB_SH1106_128X64 screen(LCD_SCL, LCD_MOSI, LCD_CS, LCD_DC, LCD_RST);
CRGB leds[NUM_LEDS];

long epoch_time;
int tempInt, tempInt1;
uint8_t monthLength, ledIndex;
long lastUpdate;
char tempChar;
bool shouldUpdate = false;

int seconds, minutes, hours, days, months, years;

char readBuffer[10];
char stringBuffer[10];
/* End Variable Definitions */

/* 
  ----------------------------------------
  |                                      |
  |            Main Functions            |
  |                                      |
  ----------------------------------------
*/
void setup() {
  if (DEBUG) Serial.begin(9600);

  initializeComponents();
  
  if (DEBUG) Serial.print("Starting WiFi Connection\n");
  wifiManager.autoConnect("Pet Feeder");

  while(WiFi.status() != WL_CONNECTED) {
    for (tempInt = 0; tempInt < NUM_LEDS; tempInt++) {
      if (tempInt % 6 == ledIndex % 6) {
        leds[tempInt] = CRGB(255,255,255);
      } else {
        leds[tempInt] = CRGB(0,0,0);
      }
    }

    FastLED.show();
    ledIndex++;
    delay(25);
  }

  if (DEBUG) Serial.print("WiFi Connected!\n");
  if (DEBUG) Serial.print("IP ");
  if (DEBUG) Serial.print(WiFi.localIP());
  if (DEBUG) Serial.print("\n");
  if (DEBUG) Serial.println(WiFi.macAddress());

  timeClient.begin();
  timeClient.setTimeOffset(-18000);
  timeClient.setUpdateInterval(UPDATE_TIME_MS);

}

void loop() {
  shouldUpdate = lastUpdate == 0 || millis() - lastUpdate > UPDATE_TIME_MS;

  if (shouldUpdate) {
    timeClient.update();
  }

  screen.firstPage();
  do {
    writeTimeToScreen(seconds, minutes, hours, readBuffer, stringBuffer);
    writeDateToScreen(days, months, years, readBuffer, stringBuffer);
    writeToScreen("Hello, World!", 0, 50);
  } while (screen.nextPage() );
  
  delay(5000);

}

/* 
  ----------------------------------------
  |                                      |
  |           Helper Functions           |
  |                                      |
  ----------------------------------------
*/
void initializeComponents() {
  if ( screen.getMode() == U8G_MODE_R3G3B2 ) {
    screen.setColorIndex(255);     // white
  }
  else if ( screen.getMode() == U8G_MODE_GRAY2BIT ) {
    screen.setColorIndex(3);         // max intensity
  }
  else if ( screen.getMode() == U8G_MODE_BW ) {
    screen.setColorIndex(1);         // pixel on
  }
  else if ( screen.getMode() == U8G_MODE_HICOLOR ) {
    screen.setHiColorByRGB(255,255,255);
  }

  screen.begin();

  FastLED.addLeds<WS2812, LED_RING, GRB>(leds, NUM_LEDS);
}

void getMonthByNumber(int monthVal, char* outArray) {
  switch (monthVal) {
    case 1:
      outArray[0] = 'J';
      outArray[1] = 'A';
      outArray[2] = 'N';
      break;
    case 2:
      outArray[0] = 'F';
      outArray[1] = 'E';
      outArray[2] = 'B';
      break;
    case 3:
      outArray[0] = 'M';
      outArray[1] = 'A';
      outArray[2] = 'R';
      break;
    case 4:
      outArray[0] = 'A';
      outArray[1] = 'P';
      outArray[2] = 'R';
      break;
    case 5:
      outArray[0] = 'M';
      outArray[1] = 'A';
      outArray[2] = 'Y';
      break;
    case 6:
      outArray[0] = 'J';
      outArray[1] = 'U';
      outArray[2] = 'N';
      break;
    case 7:
      outArray[0] = 'J';
      outArray[1] = 'U';
      outArray[2] = 'L';
      break;
    case 8:
      outArray[0] = 'A';
      outArray[1] = 'U';
      outArray[2] = 'G';
      break;
    case 9:
      outArray[0] = 'S';
      outArray[1] = 'E';
      outArray[2] = 'P';
      break;
    case 10:
      outArray[0] = 'O';
      outArray[1] = 'C';
      outArray[2] = 'T';
      break;
    case 11:
      outArray[0] = 'N';
      outArray[1] = 'O';
      outArray[2] = 'V';
      break;
    case 12:
      outArray[0] = 'D';
      outArray[1] = 'E';
      outArray[2] = 'C';
      break;
    default:
      outArray[0] = ' ';
      outArray[1] = ' ';
      outArray[2] = ' ';
  }
}
void calculateDate(long epoch_secs) {
  years = 1970;
  days = 0;
  tempInt = epoch_secs / 86400L; // number of days since epoch

  while((days += (LEAP_YEAR(years) ? 366 : 365)) <= tempInt) years++;
  tempInt -= days - (LEAP_YEAR(years) ? 366 : 365); // now it is days in this year, starting at 0
  days=tempInt;
  
  for (months=0; months<12; months++) {
    if (months==1) { // february
      monthLength = LEAP_YEAR(years) ? 29 : 28;
    } else {
      monthLength = monthDays[months];
    }
    if (days < monthLength) break;
    days -= monthLength;
  }

  days++;
  months++;
}
void calculateTime(long epoch_secs) {
  seconds = epoch_secs % 60;
  minutes = (epoch_secs % 3600) / 60;
  hours = (epoch_secs % 86400L) / 3600;
}

void writeToScreen(char* str, int x, int y) {
  screen.setFont(u8g_font_unifont);
  screen.drawStr(x,y,str);
}
void writeTimeToScreen(int currSec, int currMin, int currHour, char* buffer, char* string) {
  clearBuffer(string);
  
  if (currHour > 12) {
    string[6] = 'P';
    currHour -= 12;
  } else  {
    string[6] = 'A';
  }
  string[7] = 'M';

  if (currHour == 0) {
    currHour = 12;
  }

  itoa(currHour, buffer, 10);
  if (currHour < 10) {
    string[0] = '0';
    string[1] = buffer[0];
  } else {
    string[0] = buffer[0];
    string[1] = buffer[1];
  }

  string[2] = ':';

  itoa(currMin, buffer, 10);
  if (currMin < 10) {
    string[3] = '0';
    string[4] = buffer[0];
  } else {
    string[3] = buffer[0];
    string[4] = buffer[1];
  }

  writeToScreen(string, 0, 10);
}
void writeDateToScreen(int currDay, int currMonth, int currYear, char* buffer, char* string) {
  clearBuffer(string);

  getMonthByNumber(currMonth, buffer);
  string[0] = buffer[0];
  string[1] = buffer[1];
  string[2] = buffer[2];
  string[3] = ' ';

  itoa(currDay, buffer, 10);
  if (currDay < 10) {
    string[4] = '0';
    string[5] = buffer[0];
  } else {
    string[4] = buffer[0];
    string[5] = buffer[1];
  }

  string[6] = ' ';

  itoa(currYear, buffer, 10);
  string[7] = buffer[0];
  string[8] = buffer[1];
  string[9] = buffer[2];
  string[10] = buffer[3];

  writeToScreen(string, 0, 25);
}

void clearBuffer(char* buffer) {
  for (tempInt = 0; tempInt < (sizeof(buffer), sizeof(buffer[0])); tempInt++) {
    buffer[tempInt] = ' ';
  }
}
/* End Helper Functions */

/* 
  ----------------------------------------
  |                                      |
  |             LED Routines             |
  |                                      |
  ----------------------------------------
*/
void connectedToInternetRoutine() {
  for (tempInt = 0; tempInt < 255; tempInt++) {
    for (tempInt1 = 0; tempInt1 < NUM_LEDS; tempInt1++) {
      leds[tempInt1] = CRGB(0, tempInt, 0);
    }
    FastLED.show();
    delay(4);
  }

  for (tempInt = 255; tempInt >= 0; tempInt--) {
    for (tempInt1 = 0; tempInt1 < NUM_LEDS; tempInt1++) {
      leds[tempInt1] = CRGB(0, tempInt, 0);
    }
    FastLED.show();
    delay(6);
  }
}
/* End LED Routines */