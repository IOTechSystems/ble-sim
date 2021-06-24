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

#include "dbusutils.h"
#include "device.h"
#include "service.h"
#include "characteristic.h"
#include "descriptor.h"

#define TST_SRVC1 "0000180d-0000-1000-8000-00805f9b34fb"
#define TST_SRVC2 "0000180d-0000-1000-8000-00805f9b34f1"
#define TST_SRVC3 "0000180d-0000-1000-8000-00805f9b34f2"
#define TST_SRVC4 "0000180d-0000-1000-8000-00805f9b34f3"
#define TST_CHR1 "00002a38-0000-1000-8000-00805f9b34fb"
#define TST_CHR2 "00002a39-0000-1000-8000-00805f9b34fb"
#define TST_DESC1 "12345678-1234-5678-1234-56789abcdef2"

DBusConnection *global_dbus_connection;
char *default_adapter = NULL;

device_t *dev0 = NULL;
service_t *dev0_srvc0 = NULL;
characteristic_t *dev0_srvc0_char0 = NULL;
descriptor_t *dev0_srvc0_char0_desc0 = NULL;

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

static void init_dev (void)
{
  const char *devname = "test-dev";

  dev0 = device_new (devname, default_adapter);;
  dev0_srvc0 = service_new (TST_SRVC1, true);
  dev0_srvc0_char0 = characteristic_new (TST_CHR1);
  dev0_srvc0_char0_desc0 = descriptor_new (TST_DESC1);

  device_add_service (dev0, dev0_srvc0);
  device_add_characteristic (dev0, TST_SRVC1, dev0_srvc0_char0);
  device_add_descriptor (dev0, TST_SRVC1, TST_CHR1, dev0_srvc0_char0_desc0);

  if (!device_add (dev0))
  {
    printf ("Failed to add test device\n");
    device_free (dev0);
  }

  characteristic_set_notifying (dev0_srvc0_char0, true);

  device_set_powered (dev0, true);
  device_set_discoverable (dev0, true);
}


static int count = 0;

static void update (void *user_data)
{
  DBusConnection *connection = (DBusConnection *) user_data;
  characteristic_update_value (dev0_srvc0_char0, &count, sizeof (int), connection);
  count++;
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

static void sigint (int a)
{
  dbusutils_mainloop_running = false;
}

int main (int argc, char *argv[])
{
  signal (SIGINT, sigint);

  if (dbus_init () == false)
  {
    return 1;
  }

  default_adapter = get_default_adapter ();
  if (NULL == default_adapter)
  {
    printf ("Could not find an adapter\n");
    return 0;
  }
  printf ("Found default adapter: %s\n", default_adapter);

  init_dev ();

  dbusutils_mainloop_run (global_dbus_connection, &update);

  dbus_cleanup ();
  device_cleanup_devices ();

  printf ("Exiting\n");

  return 0;
}
