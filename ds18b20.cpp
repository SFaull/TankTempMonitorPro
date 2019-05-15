#include "ds18b20.h"
#include "config.h"

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

sensor_t mySensor[SENSOR_COUNT];

void ds18b20_init(void)
{
  sensors.begin();
  
  for (int i=0; i<SENSOR_COUNT; i++)
  {  
    sensors.getAddress(mySensor[i].address, i);
    sensors.setResolution(mySensor[i].address, RESOLUTION);
  }
  
  sensors.setWaitForConversion(false);
}

void ds18b20_request(void)
{
  for (int i=0; i<SENSOR_COUNT; i++)
  {  
    sensors.setResolution(mySensor[i].address, RESOLUTION);
  }
  
  sensors.requestTemperatures(); 
}

void ds18b20_getTemp(void)
{
  for (int i=0; i<SENSOR_COUNT; i++)
  {  
    mySensor[i].temperature = sensors.getTempCByIndex(i);
  }
}
