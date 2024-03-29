//#define DEBUG
#define ENABLE_WIRELESS
#define MQTT_JSON

/* Wireless */
#ifdef ENABLE_WIRELESS
  #include "credentials.h"
  /* Wifi */
  #define WIFI_TIMEOUT            300           // device will reset if no connection via portal within this number of seconds
  /* MQTT */
  #define MQTT_ROOM               "airing_cupboard"
  #define MQTT_TOPIC(x)           MQTT_ROOM "/" HOSTNAME "/" x
  #define MQTT_DISCOVERY_TOPIC    "homeassistant/sensor/" DEVICE_NAME 
#endif


/* General */
#define HOSTNAME                  "krusty"
#define DEVICE_NAME               HOSTNAME
#define DEVICE_MODEL              "SensorNode"
#define SERIAL_BAUD_RATE          115200
#define MQTT_PUBLISH_INTERVAL     10000 // 10 seconds
#define MQTT_DISCOVERY_INTERVAL   60000 // 60 seconds
#define BUTTON_HOLD_DURATION      800  // 0.8 seconds
/* Pinout */
#define ONE_WIRE_BUS              17  // onewire sensors are connected to pin
#define LDR_PIN                   33
#define BACKLIGHT_PWM_PIN         19
#define BUTTON_PIN                14
/* Display PWM options */
#define BACKLIGHT_PWM_CHANNEL     0
#define BACKLIGHT_PWM_FREQ        5000
#define BACKLIGHT_PWM_RES         8
/* Sensors */
#define SENSOR_READ_INTERVAL      1000  // read sensors every second
/* Display */
#define DISPLAY_REFRESH_RATE      5  // fps
#define DISPLAY_UPDATE_INTERVAL   (1000 / DISPLAY_REFRESH_RATE)
/* Temperature preferences */
#define TEMP_HOT                  40
#define TEMP_COLD                 30
#define SENSOR_COUNT 5



/* Task sensor addresses */
/* This section allows you to map any given sensor to a physical location via the sensors unique address */
#define ADDRESS_TANK_TOP          "2804D045920C026B"
#define ADDRESS_TANK_MID_HI       "28263E7997020326"
#define ADDRESS_TANK_MID_LO       "2836DC45921902DA"
#define ADDRESS_TANK_BOT          "2865F2459209021B"
#define ADDRESS_PUMP              "288C6679A20103A0"

/* ~The values below will be subtracted from the raw temperature reading */
#define OFFSET_TANK_TOP           0.4753
#define OFFSET_TANK_MID_HI       -0.3628
#define OFFSET_TANK_MID_LO        0.1583
#define OFFSET_TANK_BOT          -0.1210
#define OFFSET_PUMP               0

/* Enable averaging on the temperature measurements */
#define ENABLE_AVERAGING

/* moving average alpha value (between 0 and 1) */
#define AVG_TEMP_FILTER_FACTOR      0.2
