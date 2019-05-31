//#define DEBUG
#define ENABLE_WIRELESS

/* General */
#define DEVICE_NAME             "TankTempMonitorPro"
#define SERIAL_BAUD_RATE        115200
#define MQTT_PUBLISH_INTERVAL   5000 // 5 seconds
/* Pinout */
#define ONE_WIRE_BUS A5  // onewire sensors are connected to pin A5
/* Sensors */
#define SENSOR_READ_INTERVAL 1000  // read sensors every second
/* Display */
#define DISPLAY_UPDATE_INTERVAL 4000 // update display every second
/* Temperature preferences */
#define TEMP_HOT  30
#define TEMP_COLD 25
#define SENSOR_COUNT 5

/* Wireless */
#ifdef ENABLE_WIRELESS
  /* Wifi */
  #define WIFI_TIMEOUT            300           // device will reset if no connection via portal within this number of seconds
  /* MQTT */
  #define MQTT_ROOM               "AiringCupboard"
  #define MQTT_COMMS              MQTT_ROOM "/" DEVICE_NAME
  #define MQTT_SERVER             "your.server.here"
  #define MQTT_USERNAME           "username"
  #define MQTT_PASSWORD           "password"
  #define MQTT_PORT               16601
#endif




/* Task sesnor addresses */
/* This section allows you to map any given sensor to a physical location via the sensors unique address */
#define ADDRESS_TANK_TOP    "2804D045920C026B"
#define ADDRESS_TANK_MID_HI "28263E7997020326"
#define ADDRESS_TANK_MID_LO "2836DC45921902DA"
#define ADDRESS_TANK_BOT    "2865F2459209021B"
#define ADDRESS_PUMP        "281B3B4592060259"

/* Enable averaging on the temperature measurements */
#define ENABLE_AVERAGING

/* moving average alpha value (between 0 and 1) */
#define AVG_TEMP_FILTER_FACTOR 0.125
