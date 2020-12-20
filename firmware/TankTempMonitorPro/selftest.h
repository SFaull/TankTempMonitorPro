#include <stdint.h>

typedef enum
{
  kErrNone = 0,
  kErrSensor
} selftest_t;

void selftest_set_flag(selftest_t flag);
uint32_t selftest_get_flags(void);
bool selftest_passed(void);
