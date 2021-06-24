/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/
#include <time.h>

#include "utils.h"

bool utils_is_flag_set (unsigned int x, unsigned int flag)
{
  return x & flag;
}

void msleep (unsigned int milliseconds)
{
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep (&ts, &ts);
}
