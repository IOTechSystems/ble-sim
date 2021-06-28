/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_SERVICE_H
#define BLE_SIM_SERVICE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <dbus/dbus.h>

#include "defines.h"
#include "characteristic.h"

typedef struct service_t
{
  char *uuid; //128-bit service UUID
  char *device_path; // Object path of the Bluetooth device the service belongs to
  bool primary;
  char *object_path;
  characteristic_t *characteristics;
  unsigned int characteristic_count;
  bool lua_owned; //if the service object was created by lua - if so we should not free the service itself
  struct service_t *next;
} service_t;

/**
 * Allocates memory for a new service_t 
 * 
 * @param uuid the uuid of the service 
 * @param primary true/false if the service is the devices primary service
 * @return initialised service  
 **/
service_t *service_new (void);

/**
 * Initialises values for a new service
 * @param service service to initialise
 * @param uuid the uuid of the service 
 * @param primary true/false if the service is a devices primary service
 * @return initialised service  
 **/
service_t *service_init (service_t *service, const char *uuid, bool primary, bool lua_owned);

/**
 * Frees a service and it's values
 * 
 * @param service service to free 
 **/
void service_free (service_t *service);

/**
 * Searches the service for a characteristic
 * 
 * @param service service to search 
 * @param characteristic_uuid uuid of the characteristic
 * @return found characteristic or NULL if not found
 **/
characteristic_t *service_get_characteristic (service_t *service, const char *characteristic_uuid);

/**
 * Adds a characteristic to the service
 * 
 * @param service service to add the characteristic to
 * @param characteristic characteristic to add 
 * @return success true/false
 **/
bool service_add_characteristic (service_t *service, characteristic_t *characteristic);

//DBus methods
/**
 * Registers the service object with dbus
 * @param service pointer to the service
 * @return success true/false
 **/
bool service_register (service_t *service);

/**
 * Populates a dbus message iter with a service's object data
 * @param service pointer to the service
 * @param iter dbus message iter to populate
 **/
void service_get_object (service_t *service, DBusMessageIter *iter);

#endif //BLE_SIM_SERVICE_H
