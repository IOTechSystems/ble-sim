/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include "logger.h"

static loglevel_t log_level = DEFAULT_LOG_LEVEL;
static const char * log_level_strings[] = {"", "Info", "ERROR", "WARN", "Debug", "Trace"};

static void log_log (loglevel_t level, const char* message,va_list args);

void log_set_level (loglevel_t new_level)
{
  log_level = new_level;
}

static void log_log (loglevel_t level, const char* message, va_list args)
{
  if (level > log_level)
  {
    return;
  }

  char formatted_message[1024];
  vsnprintf (formatted_message, sizeof(formatted_message), message, args);

  time_t t = time(NULL);
  struct timeval tv;
  gettimeofday(&tv, NULL);

  char time_fmt [80];
  char time_str [80];
  strftime(time_fmt, 80, "%Y-%m-%d %H:%M:%S.%%03lu", localtime(&t));
  snprintf(time_str, sizeof (time_str), time_fmt, tv.tv_usec / 1000);

  FILE *out_stream = (level == LOG_ERROR) ? stderr : stdout;

  fprintf (out_stream, "[%s] %s: %s \n", time_str, log_level_strings[level], formatted_message);
}

void log_info (const char* message, ...)
{
  va_list args;
  va_start (args, message);
  log_log (LOG_INFO, message, args);
  va_end (args);
}

void log_error (const char* message, ...)
{
  va_list args;
  va_start (args, message);
  log_log (LOG_ERROR, message, args);
  va_end (args);
}

void log_warn (const char* message, ...)
{
  va_list args;
  va_start (args, message);
  log_log (LOG_WARN, message, args);
  va_end (args);
}

void log_debug (const char* message, ...)
{
  va_list args;
  va_start (args, message);
  log_log (LOG_DEBUG, message, args);
  va_end (args);
}

void log_trace (const char* message, ...)
{
  va_list args;
  va_start (args, message);
  log_log (LOG_TRACE, message, args);
  va_end (args);
}
