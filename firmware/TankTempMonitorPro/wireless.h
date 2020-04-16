#include <arduino.h>

typedef struct
{
  String ssid;
  String ip;
} wifiInfo_t;

void wireless_init(void);
void wireless_process(void);
void wireless_info(wifiInfo_t * info);
