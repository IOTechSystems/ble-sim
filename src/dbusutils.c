/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dbusutils.h"

bool dbusutils_mainloop_running = false;


static void dbusutils_get_object_property_data(
  DBusMessageIter *iter,
  const dbus_property_t *properties_table,
  void* object_ptr
)
{
  DBusMessageIter array;
  dbus_message_iter_open_container(
    iter,
    DBUS_TYPE_ARRAY,
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING //signature "{sv}"
    DBUS_TYPE_STRING_AS_STRING
    DBUS_TYPE_VARIANT_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
    &array
  );

  dbus_property_t *property = NULL;

  for (property = properties_table; property && property->name; property++)
  {
    printf("%s\n", property->name);
    DBusMessageIter entry, variant_container;
    dbus_message_iter_open_container(&array, DBUS_TYPE_DICT_ENTRY, NULL, &entry);


    char *tmp = strdup (property->name);
    dbus_message_iter_append_basic (&entry, DBUS_TYPE_STRING, &tmp);
    free (tmp);

    dbus_message_iter_open_container (&entry, DBUS_TYPE_VARIANT, property->signature, &variant_container);

    property->get_function(object_ptr, &variant_container);

    dbus_message_iter_close_container(&entry, &variant_container);
    dbus_message_iter_close_container(&array, &entry);
  }


  dbus_message_iter_close_container(iter, &array);
}

void dbusutils_get_object_data(
  DBusMessageIter *iter,
  const struct dbus_property_t *properties_table,
  const char* object_path,
  const char* interface,
  void* object_ptr
)
{
  DBusMessageIter entry, array, interface_entry;

  dbus_message_iter_open_container(iter, DBUS_TYPE_DICT_ENTRY, NULL, &entry);
  dbus_message_iter_append_basic(&entry, DBUS_TYPE_OBJECT_PATH, &object_path);
  dbus_message_iter_open_container(
    &entry,
    DBUS_TYPE_ARRAY,
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING //signature "{sa{sv}}"
    DBUS_TYPE_STRING_AS_STRING
    DBUS_TYPE_ARRAY_AS_STRING
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_STRING_AS_STRING
    DBUS_TYPE_VARIANT_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
    &array
  );
  dbus_message_iter_open_container (&array, DBUS_TYPE_DICT_ENTRY, NULL, &interface_entry); // open {sa{sv}}

  dbus_message_iter_append_basic (&interface_entry, DBUS_TYPE_STRING, &interface);

  dbusutils_get_object_property_data(&interface_entry, properties_table,object_ptr);

  dbus_message_iter_close_container (&array, &interface_entry);  // close {sv}
  dbus_message_iter_close_container(&entry, &array);
  dbus_message_iter_close_container(iter, &entry);

}


char *dbusutils_create_object_path(
  const char* prev_path, 
  const char* object_name,
  unsigned int object_id
)
{
  size_t required = snprintf(NULL, 0, "%s/%s%u",prev_path, object_name, object_id) + 1;
  char* path = malloc(required);
  sprintf(path, "%s/%s%u", prev_path, object_name, object_id);
  return path;
}

const char* dbusutils_get_error_message_from_reply(DBusMessage *reply)
{
  if (dbus_message_get_type(reply) != DBUS_MESSAGE_TYPE_ERROR)
  {
    return NULL;
  }

  if(strcmp(dbus_message_get_signature(reply), "s") != 0)
  {
    return NULL;
  }

  DBusMessageIter err_iter;

  const char* error_message = NULL;
  dbus_message_iter_init(reply, &err_iter);
  dbus_message_iter_get_basic(&err_iter, &error_message);

  printf("%s\n", error_message);

  return error_message;
}

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
    printf ("Error registering object path (%s): (%s)\n", object_path, err.message); 
    dbus_error_free (&err); 
    return false;
  }

  printf("Successfully registered object path %s\n", object_path);

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
