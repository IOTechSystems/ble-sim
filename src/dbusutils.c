/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <stdio.h>
#include "dbusutils.h"

bool dbusutils_mainloop_running = false;

DBusConnection * dbusutils_get_connection (void)
{
  DBusError err;
  dbus_error_init (&err);

  DBusConnection* conn = dbus_bus_get (DBUS_BUS_SYSTEM, &err);
  if (dbus_error_is_set (&err))
  { 
    printf ("Connection Error (%s)\n", err.message); 
    dbus_error_free (&err); 
  }
  return conn;
}

bool dbusutils_request_application_bus_name (DBusConnection *connection)
{
  DBusError err;
  dbus_error_init (&err);

  int ret = dbus_bus_request_name (connection, BLE_SIM_SERVICE_NAME, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
  if (dbus_error_is_set (&err)) 
  { 
      fprintf (stderr, "Name Error (%s)\n", err.message); 
      dbus_error_free (&err); 
      return false;
  }

  if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) { 
      return false;
  }
  return true;
}

bool dbusutils_register_object (DBusConnection *connection, 
                               const char* object_path, 
                               const DBusObjectPathVTable * vtable, 
                               void *user_data)
{
  DBusError err;
  dbus_error_init (&err);
  dbus_connection_try_register_object_path (connection, object_path, vtable, user_data, &err);
  if (dbus_error_is_set(&err))
  { 
    printf ("Error registering object path (%s)\n", err.message); 
    dbus_error_free (&err); 
    return false;
  }

  return true;
}

DBusMessage *dbusutils_do_method_call (DBusConnection *connection, const char *bus_name, const char *path, const char *iface, const char *method)
{
  DBusError err;
  dbus_error_init (&err);

  DBusMessage *dbus_msg = dbus_message_new_method_call (bus_name, path, iface, method);
  if (dbus_msg == NULL)
  {
    return false;
  }

  DBusMessage *dbus_reply = dbus_connection_send_with_reply_and_block (connection, dbus_msg, DEFAULT_TIMEOUT, &err);
  dbus_message_unref (dbus_msg);

  if (dbus_error_is_set (&err))
  {
    printf ("Error Sending method call (%s: %s)\n", err.name, err.message); 
    dbus_error_free (&err);
    return NULL;
  }

  dbus_error_free (&err);
  return dbus_reply;
}

static void dispatch (DBusConnection *connection)
{

  if(NULL == connection)
  {
    return;
  }

  dbus_connection_read_write (connection, 0);

  while (dbus_connection_get_dispatch_status (connection) == DBUS_DISPATCH_DATA_REMAINS )
  {
    dbus_connection_dispatch (connection);
  }
}

void dbusutils_mainloop_run (DBusConnection *connection, void (*sim_update_function_ptr)(void *) )
{
  dbusutils_mainloop_running = true;
  while (dbusutils_mainloop_running)
  {
    sim_update_function_ptr (connection);
    dispatch (connection);
  }
}
