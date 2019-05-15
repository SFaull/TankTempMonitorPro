#include "temperature.h"
#include "ds18b20.h"

#define TANK_SENSOR_COUNT 4

void temperature_init(void)
{
  ds18b20_init();
  ds18b20_request();
}

void temperature_update(void)
{
  // update values and put in the next request
  ds18b20_getTemp();
  ds18b20_request();
}

float temperature_get_tank(int index)
{
   if(index >= TANK_SENSOR_COUNT)
    return -1;

   return mySensor[index].temperature;
}

float temperature_get_pump(void)
{
  return mySensor[TANK_SENSOR_COUNT+1].temperature;
}
