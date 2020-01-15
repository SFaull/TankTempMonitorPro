#include "temperature.h"
#include "timer.h"
#include "config.h"
#include "commands.h"
#include "display.h"
#include "wireless.h"
#include "structs.h"
#include "version.h"


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
  // take sensor readings
  if(timer_expired(sensorReadTimer, SENSOR_READ_INTERVAL))
  {  
    temperature_update();
    timer_set(&sensorReadTimer);
  }

  // publish sensor readings to MQTT channel
  if(timer_expired(mqttPublishTimer, MQTT_PUBLISH_INTERVAL))
  {  
    timer_set(&mqttPublishTimer);
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
