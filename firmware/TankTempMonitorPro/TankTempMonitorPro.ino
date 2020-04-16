#include "temperature.h"
#include "timer.h"
#include "config.h"
#include "commands.h"
#include "display.h"
#include "wireless.h"
#include "structs.h"
#include "version.h"
#include "Arduino.h"
#include <Button.h>

Button button(BUTTON_PIN);
timer_t buttonHeldTimer;
timer_t sensorReadTimer;
timer_t mqttPublishTimer;
timer_t displayRefreshTimer;

void timers_init(void)
{
  // Set all timers to a large value so that all the timers fire first time around the main loop
  sensorReadTimer = 99999;
  displayRefreshTimer = 99999;

  timer_set(&mqttPublishTimer);   
  timer_set(&buttonHeldTimer);
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
  button.begin();
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
      Serial.println("Sensor update request");
      temperature_update();
      timer_set(&sensorReadTimer);
    }

       // update TFT display
    if(timer_expired(displayRefreshTimer, DISPLAY_UPDATE_INTERVAL))
    {  
      Serial.println("Display update");
      display_update();
      timer_set(&displayRefreshTimer);    
    }

    // post MQTT
    if(timer_expired(mqttPublishTimer, MQTT_PUBLISH_INTERVAL))
    {  
      Serial.println("MQTT update");
      wireless_process();
      timer_set(&mqttPublishTimer);    
    }

    // check for button press
    if(button.pressed())
    {
      timer_set(&buttonHeldTimer);
      // start timer
    }

    // check for button release
    if (button.released())
    {
        if(display_qr_mode_is_enabled())
        {
          display_qr_mode_enable(false);
        }
        else
        {
          if(timer_expired(buttonHeldTimer, BUTTON_HOLD_DURATION))
          {
            // long press action
            display_qr_mode_enable(true);
          }
          else
          {
            // short press action
            // cycle the display modes
            display_cycle_next_mode();
          }
        }
      
      displayRefreshTimer = 99999; // force a refresh by setting the timer to something large
    }
    
    wireless_process();
    commands_process();
}
