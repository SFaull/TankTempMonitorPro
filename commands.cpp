#include <SerialCommand.h>
#include <arduino.h>
#include "commands.h"
#include "version.h"
#include "config.h"
#include "temperature.h"

SerialCommand SCmd;   // The SerialCommand object

static void identify();
static void unrecognized();
static void getTemperatureTank();
static void getTemperaturePump();

void commands_init(void)
{  
  SCmd.addCommand("*IDN?", identify);
  SCmd.addCommand("TEMP:TANK", getTemperatureTank);
  SCmd.addCommand("TEMP:PUMP", getTemperaturePump);
  SCmd.addDefaultHandler(unrecognized);
}

void commands_process(void)
{
  SCmd.readSerial();
}

static void identify()
{
  Serial.print(DEVICE_NAME);
  Serial.print(", ");
  Serial.println(VERSION_STRING);
}

static void getTemperatureTank()
{
  int index;  
  char *arg; 

  arg = SCmd.next(); 
  if (arg == NULL) 
  {
    Serial.println("ERROR: invalid arg");
    return;
  }

  index=atoi(arg);
  float temp = temperature_get_tank(index);
  
  Serial.println(temp);
}

static void getTemperaturePump()
{
  float temp = temperature_get_pump();
  Serial.println(temp);
}

// This gets set as the default handler, and gets called when no other command matches. 
static void unrecognized()
{
  Serial.println("What?"); 
}
