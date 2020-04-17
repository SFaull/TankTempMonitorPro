#include <arduino.h>

typedef struct
{
  String ssid;
  String ip;
} wifiInfo_t;

void wireless_init(void);
void wireless_process(void);
void wireless_info(wifiInfo_t * info);
void wireless_mqtt_publish(void);
bool wirelss_upgrade_in_progress(unsigned int * percent);
void wireless_OTA_callback_init(void);
void wireless_forget_network(void);
