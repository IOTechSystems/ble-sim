/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <dbus/dbus.h>

#include "lua_interface.h"
#include "dbusutils.h"
#include "device.h"
#include "service.h"
#include "characteristic.h"
#include "descriptor.h"

DBusConnection *global_dbus_connection;
char *default_adapter = NULL;

char *script_path = NULL;

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

  if (!dbusutils_request_application_bus_name (global_dbus_connection))
  {
    return false;
  }

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

static char *get_default_adapter (void)
{
  int count = 0;

  DBusMessage *reply = dbusutils_do_method_call (global_dbus_connection, BLUEZ_BUS_NAME, ROOT_PATH, DBUS_INTERFACE_OBJECT_MANAGER,
                                                 DBUS_METHOD_GET_MANAGED_OBJECTS);
  if (NULL == reply)
  {
    printf ("List adapters message reply was null\n");
    return NULL;
  }
  DBusMessageIter iter, array;

  dbus_message_iter_init (reply, &iter);
  dbus_message_iter_recurse (&iter, &array);

  while (dbus_message_iter_has_next (&array)) //loop through array to get object paths
  {
    DBusMessageIter dict_entry;

    dbus_message_iter_recurse (&array, &dict_entry);
    char *object_path;
    dbus_message_iter_get_basic (&dict_entry, &object_path);


    DBusMessageIter properties;
    dbus_message_iter_next (&dict_entry);
    dbus_message_iter_recurse (&dict_entry, &properties);

    while (dbus_message_iter_has_next (&properties)) //loop through properties
    {
      DBusMessageIter property_entry;
      char *interface_name;
      dbus_message_iter_recurse (&properties, &property_entry);
      dbus_message_iter_get_basic (&property_entry, &interface_name);

      if (strcmp (BLUEZ_GATT_MANAGER_INTERFACE, interface_name) == 0)
      {
        //we are trying to return the second adapter, as the first will be used by the device service
        //once we dynamically create adapters, we can change this behaviour by creating a new adapter each time we create a "device"
        if (count == 0)
        {
          count++;
        }
        else
        {
          dbus_message_unref (reply);
          return object_path;
        }
      }
      dbus_message_iter_next (&properties);
    }
    dbus_message_iter_next (&array);
  }

  dbus_message_unref (reply);
  return NULL;
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

static void cleanup_simulator (void)
{
  device_cleanup_devices (); //cleanup devices should be called before lua cleanup
  dbus_cleanup ();
  luai_cleanup ();
}

static void stop_simulator (int a)
{
  printf ("\nStopping simulator...\n");
  dbusutils_mainloop_running = false;
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
  if (!luai_call_update ())
  {
    stop_simulator (0);
  }
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

  default_adapter = get_default_adapter ();
  if (NULL == default_adapter)
  {
    printf ("Could not find a bluetooth adapter\n");
    cleanup_simulator ();
    return 1;
  }
  printf ("Found default adapter: %s\n", default_adapter);


  if (NULL != script_path && !luai_init_state (script_path))
  {
    return 1;
  }

  signal (SIGINT, stop_simulator);
  signal (SIGTERM, stop_simulator);

  dbusutils_mainloop_run (global_dbus_connection, &update);

  cleanup_simulator ();

  return 0;
}
