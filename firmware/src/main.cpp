#include "temperature.h"
#include "timer.h"
#include "config.h"
#include "commands.h"
#include "display.h"
#include "wireless.h"
#include "structs.h"
#include "version.h"
#include "Arduino.h"
#include "JC_Button.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

void Task1code( void * pvParameters );
void Task2code( void * pvParameters );
void kickTheDog();
void timers_init(void);



TaskHandle_t Task1;
TaskHandle_t Task2;

Button button(BUTTON_PIN);       // define the button

timer_t buttonHeldTimer;
timer_t sensorReadTimer;
timer_t mqttPublishTimer;
timer_t displayRefreshTimer;



void setup() 
{
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println(DEVICE_NAME);
  Serial.println(VERSION_STRING);
  
      //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
    
}

void loop() 
{
  delay(100);
}

void kickTheDog(){
  // feed dog 0
  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable
  TIMERG0.wdt_feed=1;                       // feed dog
  TIMERG0.wdt_wprotect=0;                   // write protect
  // feed dog 1
  TIMERG1.wdt_wprotect=TIMG_WDT_WKEY_VALUE; // write enable
  TIMERG1.wdt_feed=1;                       // feed dog
  TIMERG1.wdt_wprotect=0;                   // write protect
}

void timers_init(void)
{
  // Set all timers to a large value so that all the timers fire first time around the main loop
  sensorReadTimer = 99999;
  displayRefreshTimer = 99999;

   
  timer_set(&buttonHeldTimer);
}


void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  wireless_init();
  timer_set(&mqttPublishTimer); 
   
  for(;;){

    // post MQTT
    if(timer_expired(mqttPublishTimer, MQTT_PUBLISH_INTERVAL))
    {  
      Serial.println("MQTT update");
      wireless_mqtt_publish();
      timer_set(&mqttPublishTimer);    
    }

    // if an OTA update starts, don't try and do anything else, just kick the watchdog
    unsigned int nan;
    while( wirelss_upgrade_in_progress(&nan))   {  kickTheDog(); }

    wireless_process();
    kickTheDog();
  } 
}


void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  wireless_OTA_callback_init();


  
  display_init();
  //display_splash();
  
  temperature_init();
  commands_init();
  timers_init();
  
  display_clear();
  button.begin();

  for(;;){

    display_backlight_process();
    button.read();

    // take sensor readings
    if(timer_expired(sensorReadTimer, SENSOR_READ_INTERVAL))
    {  
      Serial.println("Sensor update request");
      temperature_update();
      timer_set(&sensorReadTimer);
    }

       // update TFT display
    if(timer_expired(displayRefreshTimer, DISPLAY_UPDATE_INTERVAL))
    {  
      Serial.println("Display update");
      display_update();
      timer_set(&displayRefreshTimer);    
    }



    // check for button press
    if(button.wasPressed())
    {
      Serial.println("Button pressed");
      timer_set(&buttonHeldTimer);
      // start timer
    }

    // check for button release
    if (button.wasReleased())
    {
        Serial.println("Button released");
        if(display_qr_mode_is_enabled())
        {
          display_qr_mode_enable(false);
        }
        else
        {
          if(timer_expired(buttonHeldTimer, BUTTON_HOLD_DURATION))
          {
            // long press action
            display_qr_mode_enable(true);
          }
          else
          {
            // short press action
            display_cycle_next_mode();
          }
        }
      
      displayRefreshTimer = 99999; // force a refresh by setting the timer to something large
    }
    
    // if an OTA update starts, don't try and do anything else, just kick the watchdog
    unsigned int nan;
    while( wirelss_upgrade_in_progress(&nan))   { kickTheDog(); }

#if 0
    unsigned int upgradeProgress = 0;
    while( wirelss_upgrade_in_progress(&upgradeProgress))
    {
      static unsigned int last = 999;
      if(last != upgradeProgress)
        display_upgrade_progress(upgradeProgress);
      if(upgradeProgress == 100)
        display_upgrade_complete();
    }
#endif

    commands_process();
    kickTheDog();
  }
}
