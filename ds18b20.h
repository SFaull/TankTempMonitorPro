#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

#define RESOLUTION 12

// TODO: implement some form of rolling average
typedef struct 
{
  DeviceAddress address;
  float temperature = -1.0;
  float temperatureAve = -1.0;
  uint16_t sampleCount = 0;
} sensor_t;

extern sensor_t mySensor[SENSOR_COUNT];

void ds18b20_init(void);
void ds18b20_request(void);
void ds18b20_getTemp(void);
