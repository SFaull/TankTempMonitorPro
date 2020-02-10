#include "ds18b20.h"
#include "config.h"
#include "structs.h"

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

sensor_t mySensor[SENSOR_COUNT];

static int deviceCount;
static void printAddress(DeviceAddress deviceAddress);
static position_t getPosFromAddress(DeviceAddress deviceAddress);
uint8_t tallymarker_hextobin(const char * str, uint8_t * bytes, size_t blen);

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
    DeviceAddress address;
    position_t position;

    /* query the address and store it*/
    sensors.getAddress(address, i);
    position = getPosFromAddress(address);

    if(position == kUnknown)
    {
      Serial.println("Unknown Sensor, throwing it away");
      return;
    }

    /* store the address in the array of sensors at the position */
    sensors.getAddress(mySensor[position].address, i);
    /* set the resolution */
    sensors.setResolution(address, RESOLUTION);

    #ifdef DEBUG
      Serial.print("Sensor ");
      Serial.print(position);
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

float alpha = 0.125;

void ds18b20_getTemp(void)
{  
  for (int i=0; i<deviceCount; i++)
  {  
    //mySensor[i].temperature = sensors.getTempCByIndex(i);

    // store the raw temperature reading
    mySensor[i].temperature = sensors.getTempC(mySensor[i].address);

    if(mySensor[i].sampleCount <= 0)
      mySensor[i].temperatureAve = mySensor[i].temperature;
    
    // compute the fading memory (exponential) moving average: ma_new = alpha * new_sample + (1-alpha) * ma_old 
    mySensor[i].temperatureAve *= (1-AVG_TEMP_FILTER_FACTOR);
    mySensor[i].temperatureAve += AVG_TEMP_FILTER_FACTOR * mySensor[i].temperature;

    mySensor[i].sampleCount++;
    
    #ifdef DEBUG
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.print(", Temp: ");
      Serial.print(mySensor[i].temperature);
      Serial.print(", Avg: ");
      Serial.println(mySensor[i].temperatureAve);
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

static position_t getPosFromAddress(DeviceAddress deviceAddress)
{
  DeviceAddress top, mid_hi, mid_lo, bot, pump;

  tallymarker_hextobin(ADDRESS_TANK_TOP, top, sizeof(DeviceAddress));
  tallymarker_hextobin(ADDRESS_TANK_MID_HI, mid_hi, sizeof(DeviceAddress));
  tallymarker_hextobin(ADDRESS_TANK_MID_LO, mid_lo, sizeof(DeviceAddress));
  tallymarker_hextobin(ADDRESS_TANK_BOT, bot, sizeof(DeviceAddress));
  tallymarker_hextobin(ADDRESS_PUMP, pump, sizeof(DeviceAddress));
  

  
  if(memcmp(deviceAddress, top, sizeof(DeviceAddress)) == 0)
    return kTop;
  else if(memcmp(deviceAddress, mid_hi, sizeof(DeviceAddress)) == 0)
    return kMidHi;
  else if(memcmp(deviceAddress, mid_lo, sizeof(DeviceAddress)) == 0)
    return kMidLo;
  else if(memcmp(deviceAddress, bot, sizeof(DeviceAddress)) == 0)
    return kBottom;
  else if(memcmp(deviceAddress, pump, sizeof(DeviceAddress)) == 0)
    return kPump;
/*
  Serial.print("Unknown Address: ");
  printAddress(deviceAddress);
  Serial.println();
  Serial.print("Expected: ");
  printAddress(top);
  Serial.print(", ");
  printAddress(mid_hi);
  Serial.print(", ");
    printAddress(mid_lo);
  Serial.print(", ");
    printAddress(bot);
  Serial.print(", ");
    printAddress(pump);
  Serial.println("");
*/

  return kUnknown;
}

uint8_t tallymarker_hextobin(const char * str, uint8_t * bytes, size_t blen)
{
   uint8_t  pos;
   uint8_t  idx0;
   uint8_t  idx1;

   // mapping of ASCII characters to hex values
   const uint8_t hashmap[] =
   {
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  !"#$%&'
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ()*+,-./
     0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, // 01234567
     0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 89:;<=>?
     0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // @ABCDEFG
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // HIJKLMNO
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // PQRSTUVW
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // XYZ[\]^_
     0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00, // `abcdefg
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // hijklmno
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // pqrstuvw
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // xyz{|}~.
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // ........
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // ........
   };

    memset(bytes, 0, blen);
   //bzero(bytes, blen);
   for (pos = 0; ((pos < (blen*2)) && (pos < strlen(str))); pos += 2)
   {
      idx0 = (uint8_t)str[pos+0];
      idx1 = (uint8_t)str[pos+1];
      bytes[pos/2] = (uint8_t)(hashmap[idx0] << 4) | hashmap[idx1];
   };

   return(0);
}
