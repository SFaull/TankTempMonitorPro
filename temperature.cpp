#include "temperature.h"
#include "ds18b20.h"
#include "config.h"

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

float temperature_get(int index)
{
   if(index >= SENSOR_COUNT)
    return -1;

   return mySensor[index].temperature;
}
