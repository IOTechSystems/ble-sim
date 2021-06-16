/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <stdio.h>
#include <unistd.h>

#include "dbusutils.h"
#include "device.h"
#include "service.h"
#include "characteristic.h"
#include "descriptor.h"

DBusConnection* global_dbus_connection;

static DBusHandlerResult filter_message(DBusConnection *connection, DBusMessage *message, void *data)
{

  printf("Incomming DBus Message %d %s : %s %s/%s/%s %s\n",
          dbus_message_get_type(message),
          dbus_message_get_sender(message),
          dbus_message_get_destination(message),
          dbus_message_get_path(message),
          dbus_message_get_interface(message),
          dbus_message_get_member(message),
          dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_ERROR ?
          dbus_message_get_error_name(message) : ""
        );

  // if (dbus_message_is_method_call(msg, "test.method.Type", "Method"))
  // if (dbus_message_is_signal(msg, "test.signal.Type", "Test")) 
  
  return DBUS_HANDLER_RESULT_HANDLED;
}

static bool dbus_init(void)
{
  global_dbus_connection = dbusutils_get_connection();
  if (NULL == global_dbus_connection)
  {
    return false;
  }

  if (!dbusutils_request_application_bus_name (global_dbus_connection))
  {
    return false;
  }

  //setup filter
  if (dbus_connection_add_filter(global_dbus_connection, filter_message, NULL, NULL) == false) 
  {
    return false;
  }

  return true;
}

static void dbus_cleanup(void)
{

  if(NULL == global_dbus_connection)
  {
    return;
  }

  dbus_connection_flush(global_dbus_connection);
  dbus_connection_unref(global_dbus_connection);
}

int main(int argc, char *argv[]) 
{

  if ( dbus_init() == false)
  {
    return 1;
  }

  const char *devname = "test-dev";

  device_add(devname);

  device_add_service(devname, service_new("0000180d-0000-1000-8000-00805f9b34fb", "/path/to/dev", true, NULL) );
  device_add_service(devname, service_new("0000180d-0000-1000-8000-00805f9b34f1", "/path/to/dev", true, NULL) );
  device_add_service(devname, service_new("0000180d-0000-1000-8000-00805f9b34f2", "/path/to/dev", true, NULL) );
  device_add_service(devname, service_new("0000180d-0000-1000-8000-00805f9b34f3", "/path/to/dev", true, NULL) );

  device_add_characteristic(devname, "0000180d-0000-1000-8000-00805f9b34fb", characteristic_new("00002a38-0000-1000-8000-00805f9b34fb", "path", NULL));
  device_add_characteristic(devname, "0000180d-0000-1000-8000-00805f9b34fb", characteristic_new("00002a39-0000-1000-8000-00805f9b34fb", "path", NULL));

  device_add_descriptor(devname, "0000180d-0000-1000-8000-00805f9b34fb", "00002a39-0000-1000-8000-00805f9b34fb", descriptor_new("12345678-1234-5678-1234-56789abcdef2", "path"));

  dbus_cleanup();
  device_cleanup_devices();

  return 0;
}
