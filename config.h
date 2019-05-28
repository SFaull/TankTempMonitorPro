//#define DEBUG

/* General */
#define DEVICE_NAME             "TankTempMonitor"
#define SERIAL_BAUD_RATE        115200
#define WIFI_TIMEOUT            300           // device will reset if no connection via portal within this number of seconds

/* MQTT */
#define MQTT_PUBLISH_INTERVAL   5000 // 5 seconds
#define MQTT_ROOM               "Shed"
#define MQTT_COMMS              MQTT_ROOM "/" DEVICE_NAME

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
