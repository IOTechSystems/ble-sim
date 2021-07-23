/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include <dbus/dbus.h>

#include "bluez/src/shared/mainloop.h"

#include "lua_interface.h"
#include "dbusutils.h"
#include "device.h"
#include "service.h"
#include "characteristic.h"
#include "descriptor.h"

DBusConnection *global_dbus_connection;
char *default_adapter = NULL;
char *script_path = NULL;

pthread_t controller_mainloop_thread;

struct vhci *default_controller = NULL;

static void stop_simulator (int a);

static DBusHandlerResult filter_message (DBusConnection *connection, DBusMessage *message, void *data)
{
  printf ("Incomming DBus Message %d %s : %s %s/%s/%s %s\n",
          dbus_message_get_type (message),
          dbus_message_get_sender (message),
          dbus_message_get_destination (message),
          dbus_message_get_path (message),
          dbus_message_get_interface (message),
          dbus_message_get_member (message),
          dbus_message_get_type (message) == DBUS_MESSAGE_TYPE_ERROR ?
          dbus_message_get_error_name (message) : ""
  );

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static bool dbus_init (void)
{
  global_dbus_connection = dbusutils_get_connection ();
  if (NULL == global_dbus_connection)
  {
    return false;
  }

  dbus_bus_get_unique_name (global_dbus_connection);

  //setup filter
  if (dbus_connection_add_filter (global_dbus_connection, filter_message, NULL, NULL) == false)
  {
    return false;
  }

  return true;
}

static void dbus_cleanup (void)
{
  if (NULL == global_dbus_connection)
  {
    return;
  }

  dbus_connection_flush (global_dbus_connection);
  dbus_connection_unref (global_dbus_connection);
}

static void print_usage (const char *filename)
{
  printf ("Usage: %s [--script script_path]\n", filename);
  printf ("          [--help]\n");
}

static void print_help (const char *filename)
{
  printf ("Simulate a bluetooth low energy device\n"
          "--script/-s:\n"
          "Path to the device simulation Lua script\n");
  printf ("To simulate a device using the script register-device.lua, use the following command:\n"
          "%s -s register-device.lua\n",
          filename);
}

static bool parse_args (int argc, char *argv[])
{
  char *filename = argv[0];

  if (argc == 1)
  {
    print_usage (filename);
    return false;
  }

  for (int i = 1; i < argc; i++)
  {
    if (strcmp (argv[i], SIM_ARGS_OPTION_SCRIPT) == 0)
    {
      if (i == argc - 1)
      {
        print_usage (filename);
        return false;
      }
      i++;
      script_path = argv[i];
    }
    else if (strcmp (argv[i], SIM_ARGS_OPTION_HELP) == 0)
    {
      print_help (filename);
      return false;
    }
    else
    {
      print_usage (filename);
      return false;
    }
  }
  return true;
}

static void update (void *user_data)
{
  luai_call_update ();
}

static void *controller_mainloop_runner(void* data)
{
  mainloop_run ();
  return NULL;
}

static void cleanup_simulator (void)
{
  dbus_cleanup ();
  luai_cleanup ();
  pthread_cancel (controller_mainloop_thread);
  pthread_join (controller_mainloop_thread, NULL);
  mainloop_quit();
}

static void stop_simulator (int a)
{
  printf ("Stopping simulator...\n");
  dbusutils_mainloop_running = false;
}

int main (int argc, char *argv[])
{

  if (!parse_args (argc, argv))
  {
    return 1;
  }

  if (dbus_init () == false)
  {
    return 1;
  }

  //bluez mainloop for controllers to run on
  mainloop_init();
  pthread_create (&controller_mainloop_thread, NULL, controller_mainloop_runner, NULL);

  //create the virtual controller for the devices service to run on
  default_controller = vhci_open (VHCI_TYPE_LE);
  if (NULL == default_controller)
  {
    printf ("Could not create a virtual controller - make sure you are running as root\n");
    return 1;
  }

  printf ("Created virtual controller hci0\n");

  if (NULL == script_path || !luai_load_script (script_path))
  {
    return 1;
  }

  signal (SIGINT, stop_simulator);
  signal (SIGTERM, stop_simulator);

  dbusutils_mainloop_run (global_dbus_connection, &update);
  cleanup_simulator ();

  return 0;
}
