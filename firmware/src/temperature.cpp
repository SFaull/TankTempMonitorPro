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
   {
    Serial.print("Invalid sensor index: ");
    Serial.println(index);
    return -1;
   }

#ifdef ENABLE_AVERAGING
   return mySensor[index].temperatureAve;
#endif

  return mySensor[index].temperature;
}
