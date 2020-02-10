#include "temperature.h"
#include "timer.h"
#include "config.h"
#include "commands.h"
#include "display.h"
#include "wireless.h"
#include "structs.h"
#include "version.h"
#include "Arduino.h"

timer_t sensorReadTimer;
timer_t mqttPublishTimer;
timer_t displayRefreshTimer;

void timers_init(void)
{
  // reset timers
  timer_set(&sensorReadTimer);
  timer_set(&mqttPublishTimer);
  timer_set(&displayRefreshTimer);
}

void setup() 
{
  // configure pwm for backlight contorl
  ledcSetup(BACKLIGHT_PWM_CHANNEL, BACKLIGHT_PWM_FREQ, BACKLIGHT_PWM_RES);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(BACKLIGHT_PWM_PIN, BACKLIGHT_PWM_CHANNEL);  
  ledcWrite(BACKLIGHT_PWM_CHANNEL, 255);
  
  display_init();
  display_splash();
  
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println(DEVICE_NAME);
  Serial.println(VERSION_STRING);
  
  temperature_init();
  commands_init();
  timers_init();
  wireless_init();
  display_clear();
  display_update();
}

void loop() 
{
  /* convert light level to backlight intensity */
   int reading = analogRead(LDR_PIN);
   int brightness = map(reading, 0, 4096, 15, 255);
   ledcWrite(BACKLIGHT_PWM_CHANNEL, brightness);

    // take sensor readings
    if(timer_expired(sensorReadTimer, SENSOR_READ_INTERVAL))
    {  
      temperature_update();
      timer_set(&sensorReadTimer);
    }
  
    // update TFT display. TODO: only update if value has changed
    if(timer_expired(displayRefreshTimer, DISPLAY_UPDATE_INTERVAL))
    {  
      display_update();
      timer_set(&displayRefreshTimer);    
    }
    
    // TODO
    // check touch screen input
    // wifi process
    // OTA process
    
    commands_process();
    //wireless_process();

}
