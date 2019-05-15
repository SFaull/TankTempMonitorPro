#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>   // Hardware-specific library
void display_init(void);
void display_splash(void);
void display_refresh();
void display_update(float temp[], int size) ;
void display_update2(float temp[], int size) ;
void display_clear(void);
