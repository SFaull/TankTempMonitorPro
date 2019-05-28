#include "ds18b20.h"
#include "config.h"

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

sensor_t mySensor[SENSOR_COUNT];

static int deviceCount;
static void printAddress(DeviceAddress deviceAddress);

void ds18b20_init(void)
{
  sensors.begin();
  deviceCount = sensors.getDeviceCount();

  Serial.print("Found: ");
  Serial.print(deviceCount);
  Serial.println(" DS18B20 sensors");

  if(deviceCount != SENSOR_COUNT)
  {
    // TODO: propogate error
    Serial.print("ERROR, expected ");
    Serial.println(SENSOR_COUNT);
  }
  
  for (int i=0; i<deviceCount; i++)
  {  
    sensors.getAddress(mySensor[i].address, i);
    sensors.setResolution(mySensor[i].address, RESOLUTION);

    #ifdef DEBUG
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.print(", Res: ");
      Serial.print(sensors.getResolution(mySensor[i].address));
      Serial.print(" bits");
      Serial.print(", Address: ");
      printAddress(mySensor[i].address);
      Serial.println("");
    #endif
  }
  
  sensors.setWaitForConversion(false);
}

void ds18b20_request(void)
{    
  sensors.requestTemperatures(); 
  #ifdef DEBUG
    Serial.println("Temperature Requested");
  #endif
}

void ds18b20_getTemp(void)
{
  for (int i=0; i<deviceCount; i++)
  {  
    //mySensor[i].temperature = sensors.getTempCByIndex(i);
    mySensor[i].temperature = sensors.getTempC(mySensor[i].address);
    #ifdef DEBUG
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.print(", Temp: ");
      Serial.println(mySensor[i].temperature);
    #endif
  }
}

// function to print a device address
static void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
