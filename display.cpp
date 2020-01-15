#include "display.h"
#include "image.h"
#include "config.h"
#include "structs.h"
#include "temperature.h"
#include <TFT_eSPI.h>

#define ST7789_BLK 13 // GPIO13 (d13)

TFT_eSPI tft = TFT_eSPI();   // Invoke library
TFT_eSprite textOverlay = TFT_eSprite(&tft); // Sprite object tankInside


typedef struct 
{
  typedef
  int tank[SENSOR_COUNT-1];
  int pump;
} systemInfo_t;

static int temp2colour(float temp);

uint16_t ID;
systemInfo_t mySystem;


void display_init(void)
{
  pinMode(ST7789_BLK, OUTPUT);
  digitalWrite(ST7789_BLK, HIGH);

  tft.begin();     // initialize a ST7789 chip
  tft.setSwapBytes(true); // Swap the byte order for pushImage() - corrects endianness

  tft.fillScreen(TFT_BLACK);
}

void display_splash(void)
{
  tft.setTextSize(3);
  tft.setTextFont(2);
  tft.drawString("BOOTING....", 0, 0);
}

// TODO: improve this function so that it doesn't flash every time it is updated. The whole frame should be drawn as a sprite and then pushed
void display_update()
{  
    // Create a sprite for the text overlay
  textOverlay.setColorDepth(1);
  textOverlay.createSprite(240, 240);
  //textOverlay.fillSprite(TFT_BLACK); // Note: Sprite is filled with black when created
  textOverlay.setTextSize(3);
  textOverlay.setTextFont(2);
  textOverlay.drawFloat(temperature_get(kTop), 1, 19, 15);
  textOverlay.drawFloat(temperature_get(kMidHi), 1,19, 66);
  textOverlay.drawFloat(temperature_get(kMidLo), 1,19, 124);
  textOverlay.drawFloat(temperature_get(kBottom), 1,19, 179);
  textOverlay.drawFloat(temperature_get(kPump), 1,135, 190);

  // draw the tank graphic
  tft.fillRect(135, 190, 105, 50, TFT_BLACK);  // this is necessary to remove the pump reading from the previous frame
  tft.drawBitmap(3,4,tank_top, 115, 58, temp2colour(temperature_get(kTop)));
  tft.fillRect(3,65,115, 53, temp2colour(temperature_get(kMidHi)));
  tft.fillRect(3,121,115, 56, temp2colour(temperature_get(kMidLo)));
  tft.drawBitmap(3,180,tank_bottom, 115, 56, temp2colour(temperature_get(kBottom)));
  tft.drawBitmap(0,0,tank_outline, 121, 240, TFT_WHITE);

  tft.drawBitmap(142,120,pump_inside, 73, 64, temp2colour(temperature_get(kPump)));
  tft.drawBitmap(142,120,pump_outline, 73, 64, TFT_WHITE);
  

  // overlay the text srite we created ealier
  //tft.setBitmapColor(TFT_WHITE, TFT_BLACK); // Specify the colours of the ON and OFF pixels
  textOverlay.pushSprite(0, 0, TFT_BLACK);  // specify "BLACK" as the transparent colour

  textOverlay.deleteSprite();
}


void display_clear(void)
{
  tft.fillScreen(TFT_BLACK);
}

static int temp2colour(float temp)
{
  if(temp < 0)
    return TFT_LIGHTGREY;  // probably an error
  if(temp < TEMP_COLD)
    return TFT_BLUE;
  else if (temp < TEMP_HOT)
    return TFT_GREEN;
  else
    return TFT_RED;  
}
