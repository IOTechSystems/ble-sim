/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <stdio.h>
#include <unistd.h>

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

  device_t *new_device = device_new (devname, DEFAULT_CONTROLLER, NULL);

  device_add_service (new_device, service_new (TST_SRVC1, true, NULL));
  device_add_service (new_device, service_new (TST_SRVC2, true, NULL));
  device_add_service (new_device, service_new (TST_SRVC3, true, NULL));
  device_add_service (new_device, service_new (TST_SRVC4, true, NULL));

  device_add_characteristic (new_device, TST_SRVC1, characteristic_new (TST_CHR1, NULL));
  device_add_characteristic (new_device, TST_SRVC1, characteristic_new (TST_CHR2, NULL));

  device_add_descriptor (new_device, TST_SRVC1, TST_CHR1, descriptor_new (TST_DESC1));

  if (!device_add (new_device))
  {
    printf ("Failed to add test device\n");
    device_free (new_device);
  }
}

static void update (void *user_data)
{

}

int main (int argc, char *argv[])
{

  if (dbus_init () == false)
  {
    return 1;
  }

  init_dev ();

  dbusutils_mainloop_run (global_dbus_connection, &update);

  dbus_cleanup ();
  device_cleanup_devices ();

  return 0;
}
