typedef unsigned long timer_t;

void timer_set(timer_t *startTime);
bool timer_expired(timer_t startTime, unsigned long expiryTime);
