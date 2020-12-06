#include <SD.h>
#include <Adafruit_GFX.h>
#include <TouchScreen.h>
// LCD setup
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

// TFT setup
#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define YP A3
#define XM A2
#define YM 9
#define XP 8
#define TS_MINX 100
#define TS_MAXX 920
#define TS_MINY 70
#define TS_MAXY 900
#define STATUS_X 10
#define STATUS_Y 65
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  tft.reset();
  tft.begin(tft.readID());
  tft.setRotation(1);
  tft.fillScreen(WHITE);
  tft.setTextColor(BLACK);
  tft.setCursor(20, 78);
  tft.setTextSize(8);
  tft.setTextWrap(false);
  tft.println(F("ArduOS"));
  tft.setTextSize(2);
  tft.setCursor(23, 142); // y: 78+64
  tft.println(F("Loading, please wait..."));
  tft.setTextSize(1);
  tft.setCursor(290 ,232);
  tft.println(F("v0.01"));

  // For SD:
  pinMode(10, OUTPUT);
   
  while (!SD.begin(10)) {
    tft.setTextSize(2);
    tft.fillRect(21,140,276,20,RED);
    tft.setTextColor(WHITE);
    tft.setCursor(35, 142);
    tft.println(F("ERROR: Insert SD card"));
    tft.setTextColor(RED);
    tft.setCursor(44, 162);
    tft.println(F("Please tap to retry."));
    while(!isPressed(readTFT())) {}
    tft.fillRect(21, 140, 276, 38, WHITE);
    tft.setTextColor(BLACK);
    tft.setCursor(23, 142);
    tft.println(F("Loading, please wait..."));
  }
}

void loop() {
  // Program code
}

// unsigned int color(byte r, byte g, byte b) {
//   return 0u | ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
// }

TSPoint readTFT() {
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    // scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = (tft.height()-map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));
   }
   return p;
}

bool isPressed(TSPoint p) {
  return p.x >= 0 && p.x < tft.width() && p.y >= 0 && p.y < tft.height();
}
