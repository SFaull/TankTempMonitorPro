#include "display.h"
#include "image.h"
#include "config.h"
#include "structs.h"
#include "temperature.h"
#include <TFT_eSPI.h>
#include "qrcode.h" // https://github.com/ricmoo/QRCode
#include "wireless.h"
#include "ds18b20.h"
#include "version.h"

#define FRAME_HEIGHT  238
#define FRAME_WIDTH   238

typedef struct 
{
  typedef
  int tank[SENSOR_COUNT-1];
  int pump;
} systemInfo_t;

typedef enum {
  kTankDisplay = 0,
  kTopDisplay,
  kMidHiDisplay,
  kMidLoDisplay,
  kBottomDisplay,
  kPumpDisplay,
  kSystemInfoDisplay,
  kMaxDisplay
} displayMode_t;


static void display_QRcode_advanced(int offset_x, int offset_y, int element_size, int QRsize, int ECC_Mode, const char* Message);
static int temp2colour(float temp);

TFT_eSPI tft = TFT_eSPI();   // Invoke library
TFT_eSprite img = TFT_eSprite(&tft);  // framebuffer sprite
QRCode qrcode;

wifiInfo_t info;
static uint8_t displayMode = 0;
static bool qrMode = false;

uint16_t ID;
systemInfo_t mySystem;
static unsigned int brightness;

void display_init(void)
{
  tft.init();
  tft.fillScreen(TFT_BLACK);

  // configure pwm for backlight contorl
  ledcSetup(BACKLIGHT_PWM_CHANNEL, BACKLIGHT_PWM_FREQ, BACKLIGHT_PWM_RES);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(BACKLIGHT_PWM_PIN, BACKLIGHT_PWM_CHANNEL);  
  ledcWrite(BACKLIGHT_PWM_CHANNEL, 255);
}


void display_backlight_process()
{
    /* convert light level to backlight intensity */
   int reading = analogRead(LDR_PIN);
   brightness = map(reading, 0, 4096, 5, 255);
   ledcWrite(BACKLIGHT_PWM_CHANNEL, brightness);
}

void display_cycle_next_mode()
{
  displayMode++;
  
  // If at the last display state, loop back to the first
  if(displayMode >= kMaxDisplay) displayMode = (displayMode_t)0;

  Serial.print("Display mode: ");
  Serial.println(displayMode);

  //display_clear();
}

void display_qr_mode_enable(bool enabled)
{
  qrMode = enabled;
  if(qrMode)
  {
    wireless_info(&info); // update wifi info
    if(info.connected)
      display_QRcode("http://192.168.0.22:3000/d/yu-p1Akgk/water-tank?orgId=1&refresh=5s");
    else
    {
#if 0
      // this code will take us to the web portal
      String address = "http://" + info.ip;
      char charBuf[20];
      address.toCharArray(charBuf, address.length()+1);
      display_QRcode((char*)charBuf);
#else
      String wificonnectstring = "WIFI:T:;S:" + info.ssid + ";P:;;";
      char charBuf[60];
      wificonnectstring.toCharArray(charBuf, wificonnectstring.length()+1);
      // this code will let us join the access point and once connected, a notification will 
      display_QRcode((char*)charBuf);
#endif
    }
  }
}

bool display_qr_mode_is_enabled()
{
  return qrMode;
}

void display_splash(void)
{
  tft.setTextSize(3);
  tft.setTextFont(2);
  tft.drawString("Booting....", 0, 0);
}

void display_update()
{  
  if(qrMode)
  {
    static bool firstRun = true;
    static bool lastState = false;
    wireless_info(&info); // update wifi info
    if(firstRun)
    {
      firstRun = false;
      lastState = info.connected;
    }
    
    if(info.connected != lastState)
      display_qr_mode_enable(false);  // go back to the normal display mode

    lastState = info.connected;
    return;
  }

  img.setColorDepth(8);
  img.createSprite(FRAME_HEIGHT, FRAME_WIDTH); // FIXME: this is the largest we can set the sprite size without the pushSprite function displaying a blank frame
  img.fillSprite(TFT_BLACK); // Note: Sprite is filled with black when created
  img.setTextColor(TFT_WHITE);
  img.setTextSize(3);
  img.setTextFont(2);
  img.setCursor(0, 0);
  img.setTextDatum(TC_DATUM); // Set datum to top centre

  switch(displayMode)
  {
      case kTankDisplay:
          /* draw the tank graphic onto the display */
        Serial.println("Drawing tank");
        img.fillRect(135, 190, 105, 50, TFT_BLACK);  // this is necessary to remove the pump reading from the previous frame
        img.drawBitmap(3,4,tank_top, 115, 58, temp2colour(temperature_get(kTop)));
        img.fillRect(3,65,115, 53, temp2colour(temperature_get(kMidHi)));
        img.fillRect(3,121,115, 56, temp2colour(temperature_get(kMidLo)));
        img.drawBitmap(3,180,tank_bottom, 115, 56, temp2colour(temperature_get(kBottom)));
        img.drawBitmap(0,0,tank_outline, 121, 240, TFT_WHITE);
        img.drawBitmap(142,120,pump_inside, 73, 64, temp2colour(temperature_get(kPump)));
        img.drawBitmap(142,120,pump_outline, 73, 64, TFT_WHITE);
        
        /* draw the sensor data onto the sprite */
        Serial.println("Drawing temperatures");
        img.drawFloat(temperature_get(kTop), 1, 62, 15);
        img.drawFloat(temperature_get(kMidHi), 1,62, 66);
        img.drawFloat(temperature_get(kMidLo), 1,62, 124);
        img.drawFloat(temperature_get(kBottom), 1,62, 179);
        img.drawFloat(temperature_get(kPump), 1,182, 190);

        if(ds18b20_get_error())
        {
          int pad = 30;
          img.fillRect(pad,pad,tft.width()-(pad*2), tft.height()-(pad*2), TFT_RED);
          img.setTextSize(4);
          img.setTextDatum(MC_DATUM); // Set datum to middle
          img.setTextColor(TFT_BLACK);
          img.drawString("Sensor", tft.width()/2, (tft.height()/2-30));
          img.drawString("Error", tft.width()/2, (tft.height()/2)+30);
        }
        
      break;

      case kTopDisplay:
        img.fillSprite(temp2colour(temperature_get(kTop)));
        img.drawString("Top", tft.width()/2, 0);
        img.drawFloat(temperature_get(kTop), 1, tft.width()/2, tft.height()/2, 4);
      break;

      case kMidHiDisplay:
        img.fillSprite(temp2colour(temperature_get(kMidHi)));
        img.drawString("Uppper Mid", tft.width()/2, 0);
        img.drawFloat(temperature_get(kMidHi), 1, tft.width()/2, tft.height()/2, 4);
      break;

      case kMidLoDisplay:
        img.fillSprite(temp2colour(temperature_get(kMidLo)));
        img.drawString("Lower Mid", tft.width()/2, 0);
        img.drawFloat(temperature_get(kMidLo), 1, tft.width()/2, tft.height()/2, 4);
      break;

      case kBottomDisplay:
        img.fillSprite(temp2colour(temperature_get(kBottom)));
        img.drawString("Bottom", tft.width()/2, 0);
        img.drawFloat(temperature_get(kBottom), 1, tft.width()/2, tft.height()/2, 4);
      break;

      case kPumpDisplay:
        img.fillSprite(temp2colour(temperature_get(kPump)));
        img.drawString("Pump", tft.width()/2, 0);
        img.drawFloat(temperature_get(kPump), 1, tft.width()/2, tft.height()/2, 4);
      break;

      case kSystemInfoDisplay:
        wireless_info(&info);
        img.setTextDatum(TL_DATUM); // Set datum to top left
        img.setTextSize(2);
        img.println(VERSION_STRING);
        img.print("Sensors: "); img.println(ds18b20_get_device_count());
        img.print("Wifi: "); img.println(info.connected ? "Connected" : "Disconnected");
        img.print("SSID: " );  img.println(info.ssid);
        img.print("IP: " );  img.println(info.ip);
        img.print("Signal: "); img.println(wireless_get_connection_strength());
        img.print("Brightness: "); img.print((brightness*100)/255); img.println("%");
      break;

      case kMaxDisplay:
        // should never get here
        displayMode = kTankDisplay;
      break;
  }

  /* draw the sprite to the display as an overlay */
  Serial.println("Pushing sprite");
  img.pushSprite(1, 1);  // specify "BLACK" as the transparent colour
  img.deleteSprite();
}

void display_QRcode(const char* Message)
{
  Serial.print("Generating QR code for: ");
  Serial.println(Message);
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
        tft.fillRect(x*element_size+offset_x,y*element_size+offset_y,element_size,element_size, TFT_BLACK);
      else 
        tft.fillRect(x*element_size+offset_x,y*element_size+offset_y,element_size,element_size, TFT_WHITE);
    }
  }
}

void display_upgrade_start()
{    
  img.fillSprite(TFT_BLACK); // Note: Sprite is filled with black when created
  img.setTextColor(TFT_WHITE);
  img.setTextSize(3);
  img.setTextFont(2);
  img.drawString("Upgrading...", 5, 0);
  img.drawRoundRect(5, tft.height()/2 - 10, tft.width() - 10, 10, 10, TFT_WHITE);
  
  /* draw the sprite to the display as an overlay */
  img.pushSprite(1, 1);  // specify "BLACK" as the transparent colour
}

void display_upgrade_progress(unsigned int percent)
{    
  int progBarWidth = tft.width() - 10;
  
  img.fillSprite(TFT_BLACK); // Note: Sprite is filled with black when created
  img.setTextColor(TFT_WHITE);
  img.setTextSize(3);
  img.setTextFont(2);
  img.drawString("Upgrading...", 5, 0);
  img.drawRoundRect(5, tft.height()/2 - 10, progBarWidth, 20, 10, TFT_WHITE);
  img.fillRoundRect(5, tft.height()/2 - 10, (progBarWidth*percent)/100, 20, 10, TFT_WHITE);

  img.setCursor(5, 200);
  img.print(percent);
  img.println("%");
  
  /* draw the sprite to the display as an overlay */
  img.pushSprite(1, 1);  // specify "BLACK" as the transparent colour
}

void display_upgrade_complete()
{    
  img.fillSprite(TFT_BLACK); // Note: Sprite is filled with black when created
  img.setTextColor(TFT_WHITE);
  img.setTextSize(3);
  img.setTextFont(2);
  img.setCursor(0, 0);
  img.println("Rebooting...");
  
  img.pushSprite(1, 1);  // specify "BLACK" as the transparent colour
}

void display_upgrade_error()
{    
  img.fillSprite(TFT_BLACK); // Note: Sprite is filled with black when created
  img.setTextColor(TFT_WHITE);
  img.setTextSize(3);
  img.setTextFont(2);
  img.setCursor(0, 0);
  img.println("Upgrade");
  img.println("Error");
  
  img.pushSprite(1, 1);  // specify "BLACK" as the transparent colour
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
    return 0xAAE0;//0xEBA0;  // custom orange
  else
    return TFT_RED;  
}
