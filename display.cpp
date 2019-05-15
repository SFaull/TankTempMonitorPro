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

void display_update(float temp[], int size) 
{  
    for(int i=0; i< size; i++)
      Serial.println(temp[i]);

    int x = 10, y = 10;

    int w_bottom = 131, h_bottom = 55;
    int w_top = 131, h_top = 83;
    int w_pump = 86, h_pump = 60;

    const int SZ = 131*83 / 8;
uint8_t sram[SZ];
    
    memcpy_P(sram, water_tank_bottom, sram);

    //tft.fillScreen(BLACK);
    tft.drawBitmap(x, y+148, water_tank_bottom, w_bottom, h_bottom, temp2colour(temp[0]));    
    tft.drawBitmap(x, y+108, water_tank_bottom, w_bottom, h_bottom, temp2colour(temp[1]));
    tft.drawBitmap(x, y+68, water_tank_bottom, w_bottom, h_bottom, temp2colour(temp[2]));

    memcpy_P(sram, water_tank_top, sram);
    tft.drawBitmap(x, y, water_tank_top, w_top, h_top, temp2colour(temp[3]));

    memcpy_P(sram, pump, sram);
    tft.drawBitmap(135, 240, pump, w_pump, h_pump, temp2colour(temp[4]));
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
