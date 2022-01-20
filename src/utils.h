/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_UTILS_H
#define BLE_SIM_UTILS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Checks if a (flag) bit is set on int x 
 * @param x the integer to check if the flag is set on
 * @param flag the flag bit to check
 * @return true/false if the flag is set or not 
 */
bool utils_is_flag_set (unsigned int x, unsigned int flag);

/**
 * Sleeps for milliseconds
 * @param milliseconds milliseconds to sleep for
 **/
void msleep (unsigned int milliseconds);

#endif //BLE_SIM_UTILS_H
