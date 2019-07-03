
/* Adapted from Sermus' project : https://github.com/Sermus/ESP8266_Adafruit_ILI9341 */

#include <SPI.h>
#include <time.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library
#include <Fonts/FreeMono9pt7b.h> // Adafruit_GFX font
#include <Fonts/FreeSans9pt7b.h> // Adafruit_GFX font
#include <Fonts/FreeSerif9pt7b.h> // Adafruit_GFX font
#include "DSEG7_Classic_Regular_36pt7b.h"
#include "DSEG14_Classic_Regular_18pt7b.h"
#include "DSEG14_Classic_Regular_36pt7b.h"
#include "network_date_time_handler.h"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#if defined(ESP32)
#endif

time_t now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL); 
    return tv.tv_sec;
}

// Use SPI
Adafruit_ILI9341 * tft;

// Pins used by the ESP_WROVER_KIT V4.1
#define TFT_CS   22
#define TFT_DC   21
#define TFT_RST  18
#define TFT_BKLT  5  // Backlight is active LOW
#define SPI_SCK  19
#define SPI_MISO 25
#define SPI_MOSI 23
#define SPI_SS   -1  // Not used

//#define TARGETTEMPSCREEN

// Note: RGB888=0xA0A0A0 ; RGB555 = hex(((RGB888&0xf80000)>>8) + ((RGB888&0xfc00)>>5) + ((RGB888&0xf8)>>3)); print(RGB888, '->', RGB555);

// Based on UTFT colors
#define VGA_BLACK    0x0000
#define VGA_WHITE    0xFFFF
#define VGA_RED      0xF800
#define VGA_GREEN    0x0400
#define VGA_BLUE     0x001F
#define VGA_SILVER   0xC618
#define VGA_GRAY     0x8410
#define VGA_MAROON   0x8000
#define VGA_YELLOW   0xFFE0
#define VGA_OLIVE    0x8400
#define VGA_LIME     0x07E0
#define VGA_AQUA     0x07FF
#define VGA_TEAL     0x0410
#define VGA_NAVY     0x0010
#define VGA_FUCHSIA  0xF81F
#define VGA_PURPLE   0x8010

// Based on http://alumni.media.mit.edu/~wad/color/numbers.html
#define WAD_BLACK       0x0000  // 0x000000  // 0, 0, 0      
#define WAD_DARKGRAY    0x52aa  // 0x575757  // 87, 87, 87   
#define WAD_RED         0xa904  // 0xAD2323  // 173, 35, 35  
#define WAD_BLUE        0x2a5a  // 0x2A4BD7  // 42, 75, 215  
#define WAD_GREEN       0x1b42  // 0x1D6914  // 29, 105, 20  
#define WAD_BROWN       0x8243  // 0x814A19  // 129, 74, 25  
#define WAD_PURPLE      0x8138  // 0x8126C0  // 129, 38, 192 
#define WAD_LIGHTGRAY   0xa514  // 0xA0A0A0  // 160, 160, 160
#define WAD_LIGHTGREEN  0x862f  // 0x81C57A  // 129, 197, 122
#define WAD_LIGHTBLUE   0x9d7f  // 0x9DAFFF  // 157, 175, 255
#define WAD_CYAN        0x2e9a  // 0x29D0D0  // 41, 208, 208 
#define WAD_ORANGE      0xfc86  // 0xFF9233  // 255, 146, 51 
#define WAD_YELLOW      0xff66  // 0xFFEE33  // 255, 238, 51 
#define WAD_TAN         0xeef7  // 0xE9DEBB  // 233, 222, 187
#define WAD_PINK        0xfe7e  // 0xFFCDF3  // 255, 205, 243
#define WAD_WHITE       0xffff  // 0xFFFFFF  // 255, 255, 255

float target_room_temperature = 23.5;
float RW_temperature = 65;
float target_RW_temperature = 70;
float room1_temperature = 23.4;
float room2_temperature = 23.3;
float outside_temperature = 2.4;
float min_target_temp = 18;
float max_target_temp = 26;
bool heater_enabled = false;
time_t room1_updated = -1;
time_t room2_updated = -1;
time_t outside_updated = -1;
time_t heater_st_chg_time = 0;
time_t total_on_time = 1;
time_t total_off_time = 1;
time_t last24h_on_time = 1;
time_t last24h_off_time = 1;
time_t curr_secs;
time_t init_secs;

const uint16_t strbufSize = 255;
char strbuf [strbufSize];

//#define DEBUG

// Alignment matrix (all values not in here are invalid)
#define ALIGN_C  0x0  // b0000
#define ALIGN_W  0x1  // b0001
#define ALIGN_E  0x2  // b0010
#define ALIGN_N  0x4  // b0100
#define ALIGN_S  0x8  // b1000
#define ALIGN_NW 0x5  // b0101
#define ALIGN_NE 0x6  // b0110
#define ALIGN_SW 0x9  // b1001
#define ALIGN_SE 0xA  // b1010

#define HEADERTEXT 1
#define LINETEXT 1
#define BIGGERTEXT 2
#define BIGNUM 3

int color(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r&248)|g>>5) << 8 | ((g&28)<<3|b>>3);
}

bool DRAW_BBOX = false;

const GFXfont * defaultFontPtr = NULL;  // &FreeSerif9pt7b;
const GFXfont * tgtFontPtr = &DSEG7_Classic_Regular_36;
const GFXfont * htrFontPtr = &DSEG14_Classic_Regular_18;

int hdrSize   = 16;
int hdrRadius = 10;

void drawString(Adafruit_ILI9341 * tft, const char * textStr,
                       int16_t x=-1, int16_t y=-1, int textSize=1,
                       uint16_t fgcolor=VGA_WHITE, uint16_t bgcolor=VGA_BLACK,
                       const GFXfont * fontPtr=defaultFontPtr, uint8_t alignment=ALIGN_NW,
                       uint16_t bbox_w=0, uint16_t bbox_h=0)
{
  int16_t x1, y1;
  uint16_t min_w, min_h;
  const uint16_t debugstrSize = 255;
  char debugstr [debugstrSize];

  if (x < 0) {
    x = tft->getCursorX();
  }
  if (y < 0) {
    y = tft->getCursorX();
  }

  tft->setFont(fontPtr);
  tft->setTextSize(textSize);
  tft->getTextBounds(textStr, x, y, &x1, &y1, &min_w, &min_h);

  if (bbox_w == 0) { bbox_w = min_w; }
  if (bbox_h == 0) { bbox_h = min_h; }

  if (min_w > bbox_w || min_h > bbox_h) {
    snprintf(debugstr, debugstrSize, "Warning: bounds for \"%s\": (min_w=%d, min_h=%d) > (bbox_w=%d, bbox_h=%d)",
      textStr, min_w, min_h, bbox_w, bbox_h);
    Serial.println(debugstr);
    if (min_w > bbox_w) bbox_w = min_w;
    if (min_h > bbox_h) bbox_h = min_h;
  }

  #ifdef DEBUG
  snprintf(debugstr, debugstrSize, "getTextBounds() for string \"%s\" at (x=%d, y=%d) returned (x1=%d, y1=%d) with (min_w=%d, min_h=%d), (bbox_w=%d, bbox_h=%d)",
    textStr, x, y, x1, y1, min_w, min_h, bbox_w, bbox_h);
  Serial.println(debugstr);
  #endif

  int16_t new_x, new_y;
  switch (alignment) {
    case ALIGN_C : new_x = x + (bbox_w - min_w)/2;  // center H
                   new_y = y + (bbox_h - min_h)/2;  // center V
                   break;
    case ALIGN_W : new_x = x;
                   new_y = y + (bbox_h - min_h)/2;  // center V
                   break;
    case ALIGN_E : new_x = x + (bbox_w - min_w);
                   new_y = y + (bbox_h - min_h)/2;  // center V
                   break;
    case ALIGN_N : new_x = x + (bbox_w - min_w)/2;  // center H
                   new_y = y;
                   break;
    case ALIGN_S : new_x = x + (bbox_w - min_w)/2;  // center H
                   new_y = y + (bbox_h - min_h);
                   break;
    case ALIGN_NW: new_x = x;
                   new_y = y;
                   break;
    case ALIGN_NE: new_x = x + (bbox_w - min_w);
                   new_y = y;
                   break;
    case ALIGN_SW: new_x = x;
                   new_y = y + (bbox_h - min_h);
                   break;
    case ALIGN_SE: new_x = x + (bbox_w - min_w);
                   new_y = y + (bbox_h - min_h);
                   break;
    default      : new_x = x;
                   new_y = y;
                   break;
  }

  if (DRAW_BBOX) {
    tft->fillRect(x,         y, bbox_w, bbox_h, VGA_GREEN);
    tft->fillRect(new_x, new_y,  min_w,  min_h, VGA_BLUE);
    tft->drawRect(new_x, new_y,  min_w,  min_h, VGA_RED);
  }
  else {
    tft->fillRect(x,         y, bbox_w, bbox_h, bgcolor);
  }
  if (fontPtr) {
    tft->setCursor(new_x+x-x1, new_y+y-y1);
  }
  else {  // Default font
    tft->setCursor(new_x, new_y);
  }
  tft->setTextColor(fgcolor, bgcolor);
  tft->print(textStr);
  tft->setFont(defaultFontPtr);
}

void drawFloat(Adafruit_ILI9341 * tft, float val, int width=8, int prec=1,
               int16_t x=-1, int16_t y=-1, int textSize=1,
               uint16_t fgcolor=VGA_WHITE, uint16_t bgcolor=VGA_BLACK,
               const GFXfont * fontPtr=defaultFontPtr, uint8_t alignment=ALIGN_NW,
               uint16_t bbox_w=0, uint16_t bbox_h=0)
{
  static char outstr[32];
  drawString(tft, dtostrf(val, width, prec, outstr), x, y, textSize, fgcolor, bgcolor, fontPtr, alignment, bbox_w, bbox_h);
}

void drawInt(Adafruit_ILI9341 * tft, int val, int radix,
              int16_t x=-1, int16_t y=-1, int textSize=1,
              uint16_t fgcolor=VGA_WHITE, uint16_t bgcolor=VGA_BLACK,
              const GFXfont * fontPtr=defaultFontPtr, uint8_t alignment=ALIGN_NW,
              uint16_t bbox_w=0, uint16_t bbox_h=0)
{
  static char outstr[32];
  drawString(tft, itoa(val, outstr, radix), x, y, textSize, fgcolor, bgcolor, fontPtr, alignment, bbox_w, bbox_h);
}

int drawPlaceholder(Adafruit_ILI9341 * tft, int16_t x, int16_t y, int w, int h, uint16_t fgcolor, uint16_t bgcolor, int headersize, int radius, const char* headertext, uint8_t alignment=ALIGN_C)
{
  int interiorX = radius;
  int interiorY = 2;
  tft->fillRoundRect(x, y, w, h, radius, fgcolor);
  tft->fillRoundRect(x+interiorY, y+interiorY, w-interiorY*2, h-interiorY*2, radius, bgcolor);
  drawString(tft, headertext, x+interiorX, y+interiorY, HEADERTEXT, VGA_YELLOW, bgcolor, defaultFontPtr, alignment, w-interiorX*2, headersize);
  for (int i=0; i < interiorY; i++) {
    tft->drawFastHLine(x, y+headersize+i, w, fgcolor);
  }
  return y + headersize + interiorY;
}

const char* lvgroomSz = "Livingroom";
const char* bedroomSz = "Bedroom";
const char* outsideSz = "Outside";
const char* htrstatSz = "Heater State";
const char* uptimeSz  = "Uptime";
const char* onpcntgSz = "On Percentage";
const char* ontm24hSz = "Last 24h On";
const char* currentSz = "Current";
const char* targetSz  = "Target";

const char* targetHdr = "TARGET";
const char* heaterHdr = "HEATER";
const char* airTmpHdr = "AIR";
const char* wtrTmpHdr = "WATER";
const char* stsBdyHdr = "STATS (updated seconds ago)";

int tgtBodyOffsetV = 0;
int htrBodyOffsetV = 0;
int airBodyOffsetV = 0;
int wtrBodyOffsetV = 0;
int stsBodyOffsetV = 0;

void drawWireFrame()
{
  //Target air temperature placeholder
  tgtBodyOffsetV = drawPlaceholder(tft,   0,   0, 133, 78, VGA_RED,    VGA_BLACK, hdrSize, hdrRadius, targetHdr);
  
  //Heater icon placeholder
  htrBodyOffsetV = drawPlaceholder(tft,   0,  80, 133, 60, VGA_RED,    VGA_BLACK, hdrSize, hdrRadius, heaterHdr);

  //Air temperatures placeholder
  airBodyOffsetV = drawPlaceholder(tft, 135,   0, 184, 78, VGA_GREEN,  VGA_BLACK, hdrSize, hdrRadius, airTmpHdr);
  drawString(tft, lvgroomSz, 145, airBodyOffsetV +  5, LINETEXT, VGA_SILVER,  VGA_BLACK);
  drawString(tft, bedroomSz, 145, airBodyOffsetV + 25, LINETEXT, VGA_SILVER,  VGA_BLACK);
  drawString(tft, outsideSz, 145, airBodyOffsetV + 45, LINETEXT, VGA_SILVER,  VGA_BLACK);

  //Water temperatures placeholder
  wtrBodyOffsetV = drawPlaceholder(tft, 135,  80, 184, 60, VGA_PURPLE, VGA_BLACK, hdrSize, hdrRadius, wtrTmpHdr);
  drawString(tft, currentSz, 145, wtrBodyOffsetV +  5, LINETEXT, VGA_SILVER,  VGA_BLACK);
  drawString(tft, targetSz,  145, wtrBodyOffsetV + 25, LINETEXT, VGA_SILVER,  VGA_BLACK);

  //Statistics placeholder
  stsBodyOffsetV = drawPlaceholder(tft,   0, 142, 319, 97, VGA_WHITE, VGA_BLACK, hdrSize, hdrRadius, stsBdyHdr);
  drawString(tft, lvgroomSz,  10, stsBodyOffsetV +  5, LINETEXT, VGA_SILVER,  VGA_BLACK);
  drawString(tft, bedroomSz,  10, stsBodyOffsetV + 25, LINETEXT, VGA_SILVER,  VGA_BLACK);
  drawString(tft, outsideSz,  10, stsBodyOffsetV + 45, LINETEXT, VGA_SILVER,  VGA_BLACK);
  drawString(tft, htrstatSz, 145, stsBodyOffsetV +  5, LINETEXT, VGA_SILVER,  VGA_BLACK);
  drawString(tft, uptimeSz,  145, stsBodyOffsetV + 25, LINETEXT, VGA_SILVER,  VGA_BLACK);
  drawString(tft, onpcntgSz, 145, stsBodyOffsetV + 45, LINETEXT, VGA_SILVER,  VGA_BLACK);
  drawString(tft, ontm24hSz, 145, stsBodyOffsetV + 65, LINETEXT, VGA_SILVER,  VGA_BLACK);
}

void drawTargetTemp()
{
  uint8_t colorvalue = (target_room_temperature - min_target_temp) / (max_target_temp - min_target_temp) * 255;
  int _color = color(colorvalue, 0, 255 - colorvalue);
  snprintf(strbuf, strbufSize, "%.1f", target_room_temperature);
  drawString(tft, strbuf,  10, tgtBodyOffsetV+5, 1, _color, VGA_BLACK, tgtFontPtr, ALIGN_S, 113, 40);
}

void drawHeaterIcon()
{
  if (heater_enabled) {
    drawString(tft, "ON",  10, htrBodyOffsetV+5, 1, VGA_SILVER, VGA_BLACK, htrFontPtr, ALIGN_S, 113, 25);
  }
  else {
    drawString(tft, "OFF", 10, htrBodyOffsetV+5, 1, VGA_SILVER, VGA_BLACK, htrFontPtr, ALIGN_S, 113, 25);
  }
}

void drawAirTemperatures()
{
  drawFloat(tft, room1_temperature,     5, 1, 250, airBodyOffsetV +  5, LINETEXT, VGA_AQUA,   VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
  drawFloat(tft, room2_temperature,     5, 1, 250, airBodyOffsetV + 25, LINETEXT, VGA_AQUA,   VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
  drawFloat(tft, outside_temperature,   5, 1, 250, airBodyOffsetV + 45, LINETEXT, VGA_AQUA,   VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
}

void drawWaterTemperatures()
{
  drawFloat(tft, RW_temperature,        5, 1, 250, wtrBodyOffsetV +  5, LINETEXT, VGA_AQUA,   VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
  drawFloat(tft, target_RW_temperature, 5, 1, 250, wtrBodyOffsetV + 25, LINETEXT, VGA_AQUA,   VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
}

void drawStatistics()
{
  char uptimeStr[32];

  curr_secs = now() - init_secs;
  time_t room1_update_s = curr_secs - room1_updated;
  time_t room2_update_s = curr_secs - room2_updated;
  time_t outside_update_s = curr_secs - outside_updated;
  time_t heater_change_s = curr_secs - heater_st_chg_time;

  unsigned int days = curr_secs / 86400ul;
  unsigned int hours = (curr_secs - days * 86400ul) / 3600;
  unsigned int mins = (curr_secs - days * 86400ul - hours * 3600ul) / 60ul;
  float onpcntg = float(total_on_time)   / (total_on_time + total_off_time)     * 100;
  float ontm24h = float(last24h_on_time) / (last24h_on_time + last24h_off_time) * 100;
  snprintf(uptimeStr, COUNT_OF(uptimeStr), "%ud %02uh %02um", days, hours, mins);

  drawInt(   tft, room1_update_s,         10,  60, stsBodyOffsetV +  6, LINETEXT, VGA_AQUA, VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
  drawInt(   tft, room2_update_s,         10,  60, stsBodyOffsetV + 26, LINETEXT, VGA_AQUA, VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
  drawInt(   tft, outside_update_s,       10,  60, stsBodyOffsetV + 46, LINETEXT, VGA_AQUA, VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
  drawInt(   tft, heater_change_s,        10, 250, stsBodyOffsetV +  6, LINETEXT, VGA_AQUA, VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
  drawString(tft, uptimeStr,                  250, stsBodyOffsetV + 26, LINETEXT, VGA_AQUA, VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
  drawFloat( tft, onpcntg, 5, 1,              250, stsBodyOffsetV + 46, LINETEXT, VGA_AQUA, VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
  drawFloat( tft, ontm24h, 5, 1,              250, stsBodyOffsetV + 66, LINETEXT, VGA_AQUA, VGA_BLACK, defaultFontPtr, ALIGN_E, 60, 0);
}

void drawTargetTempScreen()
{
  uint8_t barHeight = 8;
  uint8_t barSpace = 4;
  uint8_t initialBarWidth = 8;
  int numBars = tft->height() / (barHeight + barSpace);
  for (int i = 0; i < numBars; i++)
  {
    float barTemp = min_target_temp + i * ((max_target_temp - min_target_temp) / numBars);
    int y = i * (barHeight + barSpace);
    int width = initialBarWidth + 1.0f / 1404 * y * y + 0.1624 * y;
    uint8_t colorvalue = float(i) / numBars * 255;
    int _color = color(colorvalue, 0, 255 - colorvalue);
    if (barTemp <= target_room_temperature) {
      tft->fillRoundRect(0, tft->height() - y - barHeight,     width,     barHeight,     3, _color);
    }
    else {
      tft->fillRoundRect(1, tft->height() - y - barHeight + 1, width - 2, barHeight - 2, 3, VGA_BLACK);
      tft->drawRoundRect(0, tft->height() - y - barHeight,     width,     barHeight,     3, _color);
    }
  }
  drawTargetTemp();
}

bool currentChangeTempMode = false;
void updateScreen(bool changeTempMode)
{
#ifdef TARGETTEMPSCREEN
  if (currentChangeTempMode != changeTempMode) {
    tft->fillScreen(VGA_BLACK);
    if (!changeTempMode) {
      drawWireFrame();
      drawTargetTemp();
    }
  }
  if (changeTempMode) {
    drawTargetTempScreen();
  }
  else {
#endif
    drawTargetTemp();
    drawAirTemperatures();
    drawWaterTemperatures();
    drawHeaterIcon();
    drawStatistics();
#ifdef TARGETTEMPSCREEN
  } 
  currentChangeTempMode = changeTempMode;
#endif
}

void setupUI() {
  init_secs = now();
  curr_secs = init_secs;
  tft->setFont(defaultFontPtr);
  tft->setTextColor(VGA_GREEN, VGA_BLACK);
  tft->fillScreen(VGA_BLACK);
  drawWireFrame();
  drawTargetTemp();
  updateScreen(false);
}

void test_screen() {
    int test_colors [] = {
    WAD_WHITE,
    WAD_RED,
    WAD_ORANGE,
    WAD_YELLOW,
    WAD_GREEN,
    WAD_BLUE,
    WAD_PURPLE,
    WAD_BLACK,
  };
  for (int n = 0; n < COUNT_OF(test_colors); n++) {
    tft->fillScreen(test_colors[n]);
    delay(100);
  }
}

void test_fonts() {
  DRAW_BBOX = true;
  tft->drawRect(0, 0, tft->width(), tft->height(), VGA_RED);
  tft->fillRect(1, 1, tft->width()-2, tft->height()-2, VGA_BLACK);
  drawString(tft, "12345.67890", 10,  10, 1, VGA_YELLOW, VGA_BLUE, &DSEG7_Classic_Regular_36,  ALIGN_C, 300, 105);
  drawString(tft, "12345.67890", 10, 120, 1, VGA_YELLOW, VGA_BLUE, &DSEG14_Classic_Regular_36, ALIGN_C, 300, 105);
  DRAW_BBOX = false;
  delay(1000);
}

void test_alignment() {
  DRAW_BBOX = true;
  tft->drawRect(0, 0, tft->width(), tft->height(), VGA_RED);
  tft->fillRect(1, 1, tft->width()-2, tft->height()-2, VGA_BLACK);
  drawString(tft, "#nw#",   8,  10, 2, VGA_YELLOW, VGA_BLUE, defaultFontPtr, ALIGN_NW, 96, 68);
  drawString(tft, "#w#",    8,  86, 2, VGA_YELLOW, VGA_BLUE, defaultFontPtr, ALIGN_W,  96, 68);
  drawString(tft, "#sw#",   8, 162, 2, VGA_YELLOW, VGA_BLUE, defaultFontPtr, ALIGN_SW, 96, 68);
  drawString(tft, "#n#",  112,  10, 2, VGA_YELLOW, VGA_BLUE, defaultFontPtr, ALIGN_N,  96, 68);
  drawString(tft, "#cc#", 112,  86, 2, VGA_YELLOW, VGA_BLUE, defaultFontPtr, ALIGN_C,  96, 68);
  drawString(tft, "#s#",  112, 162, 2, VGA_YELLOW, VGA_BLUE, defaultFontPtr, ALIGN_S,  96, 68);
  drawString(tft, "#ne#", 216,  10, 2, VGA_YELLOW, VGA_BLUE, defaultFontPtr, ALIGN_NE, 96, 68);
  drawString(tft, "#e#",  216,  86, 2, VGA_YELLOW, VGA_BLUE, defaultFontPtr, ALIGN_E,  96, 68);
  drawString(tft, "#se#", 216, 162, 2, VGA_YELLOW, VGA_BLUE, defaultFontPtr, ALIGN_SE, 96, 68);
  DRAW_BBOX = false;
  delay(1000);
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("HVAC Demo v2");

  initNetworkTime();
  Serial.println(strLocalDateTime());

  Serial.println();

  #define TFT_CS   22
  #define TFT_DC   21
  #define TFT_RST  18 // Reset pin
  #define TFT_BKLT 5
  SPIClass * hspi = new SPIClass(HSPI);
  hspi->begin(19, 25, 23, -1);
  tft = new Adafruit_ILI9341(hspi, TFT_DC, TFT_CS, TFT_RST);
  #if defined(TFT_BKLT)
    pinMode(TFT_BKLT, OUTPUT);
    digitalWrite(TFT_BKLT, HIGH);
  #endif

  tft->begin();
  tft->setRotation(1);
  tft->fillScreen(VGA_BLACK);

  #if defined(TFT_BKLT)
    pinMode(TFT_BKLT, OUTPUT);
    digitalWrite(TFT_BKLT, LOW);
  #endif

  #ifdef SELF_TEST
    test_screen();
    test_fonts();
    test_alignment();
  #endif

  Serial.println("LCD_init done....\n");  

  setupUI();
}


uint32_t startTime, elapsedTime;

void loop() {
  startTime = millis();

  if (heater_enabled) {
      target_room_temperature += 0.1;
      if (target_room_temperature >= 30.0) {
        heater_enabled = false;
        heater_st_chg_time = curr_secs;
      }
    }
    else {
      target_room_temperature -= 0.1;         
      if (target_room_temperature <= 20.0) {
        heater_enabled = true;
        heater_st_chg_time = curr_secs;
      }
  }
  updateScreen(false);

  elapsedTime = millis()-startTime;
  Serial.print("Took "); Serial.print(elapsedTime); Serial.println(" ms");
  delay(955);
}
