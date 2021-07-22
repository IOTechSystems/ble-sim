/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_LOGGER_H
#define BLE_SIM_LOGGER_H

#include <stdarg.h>
#include <stdbool.h>

typedef enum loglevel_t
{
  LOG_NONE = 0,   
  LOG_INFO,
  LOG_ERROR,      
  LOG_WARN,          
  LOG_DEBUG,      
  LOG_TRACE      
} loglevel_t;

#define DEFAULT_LOG_LEVEL LOG_WARN;

void log_set_level (loglevel_t new_level);

void log_info(const char* format, ...);

void log_error(const char* format, ...);

void log_warn(const char* format, ...);

void log_debug(const char* format, ...);

void log_trace(const char* format, ...);


#endif //BLE_SIM_LOGGER_H
