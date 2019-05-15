#include "display.h"
#include "image.h"
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

MCUFRIEND_kbv tft;
uint16_t ID;

void display_init(void)
{
  ID = tft.readID();
  if (ID == 0xD3) ID = 0x9481;
  tft.begin(ID);
  tft.setRotation(2);
}

void display_update(float t0, float t1, float t2, float t3, float tpump)
{
  //tft.fillScreen(BLACK);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println("Temperatures:");
  tft.println(t0);
  tft.println(t1);
  tft.println(t2);
  tft.println(t3);
  tft.println(tpump);
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

// TODO interpret temperatures to select colour
void display_drawTank(void)
{
    int x = 5, y, w = 131, h = 83;
    uint32_t t;
    const int SZ = w * h / 8;
    uint8_t sram[SZ];
    memcpy_P(sram, water_tank_white, SZ);

    tft.fillScreen(BLACK);
    tft.drawBitmap(x, y+120, water_tank_white, 131, 83, RED);
    tft.drawBitmap(x, y+80, water_tank_white, 131, 83, BLUE);
    tft.drawBitmap(x, y+40, water_tank_white, 131, 83, BLUE);
    tft.drawBitmap(x, y, water_tank_white, 131, 83, GREEN);
    
}
