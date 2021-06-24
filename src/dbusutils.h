/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_DBUSUTILS_H
#define BLE_SIM_DBUSUTILS_H

#include <stdbool.h>
#include <dbus/dbus.h>

#include "defines.h"

extern DBusConnection *global_dbus_connection;

typedef void(*dbus_get_object_property_function) (void *user_data, DBusMessageIter *iter);

typedef bool(*dbus_call_object_method_function) (void *user_data, DBusConnection *connection, DBusMessage *message);

typedef struct dbus_property_t
{
  const char *name;
  const char *signature;
  dbus_get_object_property_function get_function;
} dbus_property_t;

typedef struct dbus_method_t
{
  const char *interface;
  const char *method;
  dbus_call_object_method_function object_method_function;
} dbus_method_t;

typedef struct object_flag_t
{
  const char *flag_value;
  unsigned int enabled_bit;
} object_flag_t;

/**
 * Appends a dict entry which contains an array to a dbus message iter
 * @param iter dbus message iter to append to 
 * @param key_type the key type
 * @param key ptr to the key's value
 * @param val_type the arrays element type
 * @param val ptr to the array
 * @param elements number of elements in the array 
 **/
void dbusutils_iter_append_dict_entry_fixed_array (
  DBusMessageIter *iter,
  int key_type,
  const void *key,
  int val_type,
  const void *val,
  unsigned int elements
);

/**
 * Appends a string to a dbus iter - trying to append a string literal causes a segfault
 * so this function allows the use of string literals
 * 
 * @param iter a pointer to the the dbus message iter to append to 
 * @param type the type to append - can be either DBUS_TYPE_STRING or DBUS_TYPE_OBJECT_PATH
 * @param string the string to append
 **/
void dbusutils_iter_append_string (DBusMessageIter *iter, int type, const char *string);

/**
 * Populates a dbus message iterator with object data based on a property table
 * 
 * @param iter a pointer to the dbus message iterator to populate with values
 * @param properties_table a NULL terminated properties table
 * @param object_path the object path
 * @param interface the interface the object implements
 * @param object_pointer a pointer to the object that will be passed to the functions in the properties table 
 **/
void dbusutils_get_object_data (
  DBusMessageIter *iter,
  dbus_property_t *properties_table,
  const char *object_path,
  const char *interface,
  void *object_pointer
);

/**
 * creates a valid dbus object path:
 * prev_path + "/" + object_name + object_id
 * 
 * e.g prev_path = "/dev", object_name = "service", object_id = 1
 *  output would be "/dev/service1"
 * 
 * @param prev_path object hierarchy path
 * @param object_name name of the object
 * @param object_id id of the object
 * @return the created object path
 **/
char *dbusutils_create_object_path (
  const char *prev_path,
  const char *object_name,
  unsigned int object_id
);

/**
 * @param The dbus message to get the error message from
 * @return error message (should not be free'd) or NULL
 **/
const char *dbusutils_get_error_message_from_reply (DBusMessage *reply);

/**
 * Creates a dbus connection
 * 
 * @return a new DBusConnection or NULL if one could not be created
 **/
DBusConnection *dbusutils_get_connection (void);

/**
 *  Asks DBus to assign IOTECH_BLE_SIM_SERVICE_NAME to a connection 
 *  @param DBusConnection the dbus connection 
 *  @return true if successful, false if not
 **/
bool dbusutils_request_application_bus_name (DBusConnection *connection);

/**
 *  Registers a handler for a given path in the object hierarchy. 
 *  @param DBusConnection the dbus connection 
 *  @param path the object path
 *  @param property_table the object's property table
 *  @param method_table the object's method table
 *  @param object_ptr the pointer to the object
 *  @return successful true/false
 **/
bool dbusutils_register_object (DBusConnection *connection, const char *path, dbus_property_t *property_table, dbus_method_t *method_table, void *object_ptr);

/**
 * Performs a dbus method call
 *
 * @param dbus_conn a DBusConnection
 * @param bus_name Dbus bus name
 * @param path dbus path
 * @param iface dbus interface
 * @param method dbus method to call
 * @return the method call reply 
 */
DBusMessage *dbusutils_do_method_call (DBusConnection *dbus_conn, const char *bus_name, const char *path, const char *iface, const char *method);

/**
 * Performs a dbus method call to set a property
 *
 * @param dbus_conn a DBusConnection
 * @param bus_name Dbus bus name
 * @param path dbus path
 * @param iface dbus interface
 * @param property the name of the property we are setting
 * @param type the dbus data type
 * @param data pointer to the data
 * @return the method call reply 
 */
DBusMessage *dbusutils_set_property_basic (
  DBusConnection *connection,
  const char *bus_name,
  const char *path,
  const char *iface,
  const char *property,
  int data_type,
  void *data
);


extern bool dbusutils_mainloop_running;

/**
 * Starts the main DBus read/write/dispatch loop
 * 
 * @param connection a DBusConnection
 * @param sim_update_function_ptr function pointer that is run in the main loop
 * @return void
 **/
void dbusutils_mainloop_run (DBusConnection *connection, void (*sim_update_function_ptr) (void *));

#endif //BLE_SIM_DBUSUTILS_H
