#include "temperature.h"
#include "timer.h"
#include "config.h"
#include "commands.h"
#include "display.h"

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
  temperature_init();
  commands_init();
  timers_init();
   display_drawTank();
   while(1){}
}

void loop() 
{
  // take sensor readings
  if(timer_expired(sensorReadTimer, SENSOR_READ_INTERVAL))
  {  
    timer_set(&sensorReadTimer);
    temperature_update();
  }

  // publish sensor readings to MQTT channel
  if(timer_expired(mqttPublishTimer, MQTT_PUBLISH_INTERVAL))
  {  
    timer_set(&mqttPublishTimer);
  }

  // update TFT display. TODO: only update if value has changed
  if(timer_expired(displayRefreshTimer, DISPLAY_UPDATE_INTERVAL))
  {  
    timer_set(&displayRefreshTimer);    
    display_update(temperature_get_tank(0),temperature_get_tank(1),temperature_get_tank(2),temperature_get_tank(3),temperature_get_pump());
  }
  
  // TODO
  // check touch screen input
  // wifi process
  // OTA process
  
  commands_process();
}
