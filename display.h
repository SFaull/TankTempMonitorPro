#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>   // Hardware-specific library

void display_init(void);
void display_splash(void);
void display_refresh();
void display_update(float t0, float t1, float t2, float t3, float tpump);
void display_drawTank(void);
