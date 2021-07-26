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

/**
 * Sets the logging level from a string
 * @param str log level str
 * @return success true/false
 */
bool log_set_level_from_str (const char *str);

/**
 * Sets the logging level
 * @param new_level the new logging level
 */
void log_set_level (loglevel_t new_level);

/**
 * Logs an info message
 * @param format format string to print
 * @param ... Args to the format string
 */
void log_info(const char* format, ...);

/**
 * Logs an error message
 * @param format format string to print
 * @param ... Args to the format string
 */
void log_error(const char* format, ...);

/**
 * Logs a warning message
 * @param format format string to print
 * @param ... Args to the format string
 */
void log_warn(const char* format, ...);

/**
 * Logs a debug message
 * @param format format string to print
 * @param ... Args to the format string
 */
void log_debug(const char* format, ...);

/**
 * Logs a trace message
 * @param format format string to print
 * @param ... Args to the format string
 */
void log_trace(const char* format, ...);

#endif //BLE_SIM_LOGGER_H
