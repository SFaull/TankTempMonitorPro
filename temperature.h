#define TANK_SENSOR_COUNT 4

void temperature_init(void);
void temperature_update(void);
float temperature_get_tank(int index);
float temperature_get_pump(void);
