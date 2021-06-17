/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_SERVER_H
#define BLE_SIM_SERVER_H

#include <dbus/dbus.h>

#include "defines.h"
#include "service.h"

//TODO move this into device

typedef struct server_t
{
  char * object_path; //dbus object path
  service_t *services;
  
} server_t;

server_t *server_new (const char *object_path, service_t *services);
void server_free (server_t *server);

bool server_add_service (server_t *server, service_t *service);
service_t* server_get_service (server_t *server, const char *service_uuid);

//DBus methods
void server_get_managed_objects (DBusConnection *connection, DBusMessage *message, server_t *device);

#endif //BLE_SIM_SERVER_H
