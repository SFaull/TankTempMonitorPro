#include "timer.h"
#include <arduino.h>
/* pass this function a pointer to an unsigned long to store the start time for the timer */
void timer_set(timer_t *startTime)
{
  *startTime = millis();  // store the current time
}

/* call this function and pass it the variable which stores the timer start time and the desired expiry time 
   returns true fi timer has expired */
bool timer_expired(timer_t startTime, unsigned long expiryTime)
{
  if( (millis() - startTime) >= expiryTime )
    return true;
  return false;
}
