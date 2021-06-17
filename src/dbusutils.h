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

/**
 * Creates a dbus connection
 * 
 * @return a new DBusConnection or NULL if one could not be created
 **/
DBusConnection * dbusutils_get_connection (void);

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
 *  @param vtable the callback table
 *  @param user_data data to pass to functions in the callback table
 *  @return successful true/false
 **/
bool dbusutils_register_object (DBusConnection *connection, const char* path, const DBusObjectPathVTable * vtable, void *user_data);

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
DBusMessage * dbusutils_do_method_call (DBusConnection *dbus_conn, const char *bus_name, const char *path, const char *iface, const char *method);

extern bool dbusutils_mainloop_running;

/**
 * Starts the main DBus read/write/dispatch loop
 * 
 * @param connection a DBusConnection
 * @param sim_update_function_ptr function pointer that is run in the main loop
 * @return void
 **/
void dbusutils_mainloop_run (DBusConnection *connection, void (*sim_update_function_ptr)(void *));

#endif //BLE_SIM_DBUSUTILS_H
