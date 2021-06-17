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

static bool dbus_init (void)
{
  global_dbus_connection = dbusutils_get_connection ();
  if (NULL == global_dbus_connection)
  {
    return false;
  }

  if (!dbusutils_request_application_bus_name (global_dbus_connection) )
  {
    return false;
  }
 
  //setup filter
  // if (dbus_connection_add_filter (global_dbus_connection, filter_message, NULL, NULL) == false) 
  // {
  //   return false;
  // }

  return true;
}

static void dbus_cleanup (void)
{

  if(NULL == global_dbus_connection)
  {
    return;
  }

  dbus_connection_flush (global_dbus_connection);
  dbus_connection_unref (global_dbus_connection);
}


static void init_dev(void)
{
  const char *devname = "test-dev";
  device_add (devname);
  device_add_service (devname, service_new("0000180d-0000-1000-8000-00805f9b34fb", "/path/to/dev", true, NULL) );
  device_add_service (devname, service_new("0000180d-0000-1000-8000-00805f9b34f1", "/path/to/dev", true, NULL) );
  device_add_service (devname, service_new("0000180d-0000-1000-8000-00805f9b34f2", "/path/to/dev", true, NULL) );
  device_add_service (devname, service_new("0000180d-0000-1000-8000-00805f9b34f3", "/path/to/dev", true, NULL) );

  device_add_characteristic (devname, "0000180d-0000-1000-8000-00805f9b34fb", characteristic_new ("00002a38-0000-1000-8000-00805f9b34fb", "path", NULL));
  device_add_characteristic (devname, "0000180d-0000-1000-8000-00805f9b34fb", characteristic_new ("00002a39-0000-1000-8000-00805f9b34fb", "path", NULL));

  device_add_descriptor (devname, "0000180d-0000-1000-8000-00805f9b34fb", "00002a39-0000-1000-8000-00805f9b34fb", descriptor_new ("12345678-1234-5678-1234-56789abcdef2", "path"));
}

static void update(void * user_data)
{
  DBusMessage *reply = dbusutils_do_method_call(global_dbus_connection, IOTECH_BLE_SIM_SERVICE_NAME, "/dev0", DBUS_INTERFACE_OBJECT_MANAGER, DBUS_METHOD_GET_MANAGED_OBJECTS);
  if (NULL == reply)
  {
    return;
  }
  
  dbus_message_unref(reply);
}

int main (int argc, char *argv[]) 
{

  if ( dbus_init () == false)
  {
    return 1;
  }

  init_dev();

  dbusutils_mainloop_run(global_dbus_connection, &update);  
  
  dbus_cleanup ();
  device_cleanup_devices ();

  return 0;
}
