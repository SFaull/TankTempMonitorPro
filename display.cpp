#include "display.h"
#include "image.h"
#include "config.h"
/*
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <FreeDefaultFonts.h>

#define LCD_CS    A3
#define LCD_CD    A2
#define LCD_WR    A1
#define LCD_RD    A0
#define LCD_RESET A4
*/

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREY    0x8410
#define ORANGE  0xE880

typedef struct 
{
  typedef
  int tank[SENSOR_COUNT-1];
  int pump;
} systemInfo_t;

static int temp2colour(float temp);
static void print_float_at(float val, int x, int y);

static void updateTankTop(float temp);
static void updateTankBottom(float temp1, float temp2, float temp3);
static void updatePump(float temp);

MCUFRIEND_kbv tft;
uint16_t ID;
systemInfo_t mySystem;


void display_init(void)
{
  ID = tft.readID();
  if (ID == 0xD3) ID = 0x9481;
  tft.begin(ID);
  tft.setRotation(2);
}

void display_update2(float temp[], int size)
{
    print_float_at(temp[3], 150, 50);
    print_float_at(temp[2], 150, 90);
    print_float_at(temp[1], 150, 130);
    print_float_at(temp[0], 150, 170);
    print_float_at(temp[4], 50, 260);
}

// FIXME: something very wrong with this function, often causes crash
void display_update(float temp[], int size) 
{  
    updateTankBottom(temp[0], temp[1], temp[2]);
    updateTankTop(temp[3]);
    updatePump(temp[4]);
}

static void updateTankTop(float temp)
{
    int w = 131, h = 83;
    int x = 10, y = 10;
    tft.drawBitmap(x, y, water_tank_top, w, h, temp2colour(temp));
}

static void updateTankBottom(float temp1, float temp2, float temp3)
{
  int w = 131, h = 55;
  int x = 10, y = 10;
  tft.drawBitmap(x, y+148, water_tank_bottom, w, h, temp2colour(temp1));    
  tft.drawBitmap(x, y+108, water_tank_bottom, w, h, temp2colour(temp2));
  tft.drawBitmap(x, y+68, water_tank_bottom, w, h, temp2colour(temp3));
}

static void updatePump(float temp)
{
  int w = 86, h = 60;
  int x = 10, y = 10;
  tft.drawBitmap(135, 240, pump, w, h, temp2colour(temp));
}

void display_refresh(void)
{  
  tft.fillScreen(BLACK);
  tft.setTextColor(GREEN, BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.print("Ready...");
}

void display_splash(void)
{
  tft.fillScreen(BLACK);
  tft.setTextColor(RED, BLACK);
  tft.setTextSize(3);
  tft.setCursor(0, 0);
  tft.print("Booting up...");
}


void display_clear(void)
{
  tft.fillScreen(BLACK);
}

static int temp2colour(float temp)
{
  if(temp < 0)
    return GREY;  // probably an error
  if(temp < TEMP_COLD)
    return BLUE;
  else if (temp < TEMP_HOT)
    return GREEN;
  else
    return RED;  
}

static void print_float_at(float val, int x, int y)
{
    char buf[10];
    int16_t x1, y1, w, h;
    tft.getTextBounds("000.0", x, y, &x1, &y1, &w, &h); 
    dtostrf(val, 5, 1, buf);   //e.g. 123.4
    tft.fillRect(x1, y1, w, h, BLACK);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.setCursor(x, y);
    tft.print(buf);
}
