#include "display.h"
#include "image.h"
#include "config.h"
#include "structs.h"
#include "temperature.h"
#include <TFT_eSPI.h>
#include "qrcode.h" // https://github.com/ricmoo/QRCode

TFT_eSPI tft = TFT_eSPI();   // Invoke library
TFT_eSprite textOverlay = TFT_eSprite(&tft); // Sprite object tankInside
QRCode qrcode;

typedef struct 
{
  typedef
  int tank[SENSOR_COUNT-1];
  int pump;
} systemInfo_t;

static void display_QRcode_advanced(int offset_x, int offset_y, int element_size, int QRsize, int ECC_Mode, const char* Message);
static int temp2colour(float temp);

uint16_t ID;
systemInfo_t mySystem;


void display_init(void)
{
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

void display_QRcode(const char* Message)
{
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(12,12,215,215, TFT_WHITE);
  display_QRcode_advanced(17,17,5,6,2,Message);
}

void display_QRcode_advanced(int offset_x, int offset_y, int element_size, int QRsize, int ECC_Mode, const char* Message)
{
  uint8_t qrcodeData[qrcode_getBufferSize(QRsize)];
  //ECC_LOW, ECC_MEDIUM, ECC_QUARTILE and ECC_HIGH. Higher levels of error correction sacrifice data capacity, but ensure damaged codes remain readable.
  if (ECC_Mode%4 == 0) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_LOW, Message);
  if (ECC_Mode%4 == 1) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_MEDIUM, Message);
  if (ECC_Mode%4 == 2) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_QUARTILE, Message);
  if (ECC_Mode%4 == 3) qrcode_initText(&qrcode, qrcodeData, QRsize, ECC_HIGH, Message);
  
  for (int y = 0; y < qrcode.size; y++) 
  {
    for (int x = 0; x < qrcode.size; x++) 
    {
      if (qrcode_getModule(&qrcode, x, y)) 
      {
        tft.fillRect(x*element_size+offset_x,y*element_size+offset_y,element_size,element_size, TFT_BLACK);
      }
      else 
      {
        tft.fillRect(x*element_size+offset_x,y*element_size+offset_y,element_size,element_size, TFT_WHITE);
      }
    }
  }
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
