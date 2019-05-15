#include "timer.h"
#include <arduino.h>
/* pass this function a pointer to an unsigned long to store the start time for the timer */
void timer_set(unsigned long *startTime)
{
  *startTime = millis();  // store the current time
}

/* call this function and pass it the variable which stores the timer start time and the desired expiry time 
   returns true fi timer has expired */
bool timer_expired(unsigned long startTime, unsigned long expiryTime)
{
  if( (millis() - startTime) >= expiryTime )
    return true;
  return false;
}
