/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/
#include "utils.h"

bool utils_is_flag_set(unsigned int x, unsigned int flag)
{
  return x & flag;
}
