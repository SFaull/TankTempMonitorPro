#include "selftest.h"

static uint32_t selftestFlags = 0x00000000;

void selftest_set_flag(selftest_t flag)
{
  selftestFlags |= (1 << (uint32_t)flag);
}

uint32_t selftest_get_flags()
{
  return selftestFlags;
}

bool selftest_passed()
{
  return (selftestFlags == 0);
}
