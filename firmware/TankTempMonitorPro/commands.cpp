#include <SerialCommand.h>
#include <arduino.h>
#include "commands.h"
#include "version.h"
#include "config.h"
#include "temperature.h"
#include "wireless.h"

SerialCommand SCmd;   // The SerialCommand object

static void identify();
static void unrecognized();
static void getTemperature();
static void networkForget();
static void unrecognized();

void commands_init(void)
{  
  SCmd.addCommand("*IDN?", identify);
  SCmd.addCommand("TEMP?", getTemperature);
  SCmd.addCommand("NETWORK:FORGET", networkForget);
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

static void networkForget()
{
  wireless_forget_network();
}


static void getTemperature()
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
  float temp = temperature_get(index);
  
  Serial.println(temp);
}

// This gets set as the default handler, and gets called when no other command matches.
static void unrecognized() {
  Serial.println("What?");
}
