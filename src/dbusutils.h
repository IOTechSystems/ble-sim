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

DBusConnection * dbusutils_get_connection(void);
bool dbusutils_request_application_bus_name (DBusConnection *connection);

bool dbusutils_register_object(DBusConnection *connection, const char* path, const DBusObjectPathVTable * vtable, void *user_data);
bool dbusutils_do_method_call (DBusConnection *dbus_conn, const char *bus_name, const char *path, const char *iface, const char *method);

extern bool dbusutils_mainloop_running;

void dbusutils_mainloop_run (DBusConnection *connection, void (*sim_update_function_ptr)(void *));

#endif //BLE_SIM_DBUSUTILS_H
