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

#include "defines.h"
#include "characteristic.h"

typedef struct service_t
{
  char *uuid; //128-bit service UUID
  char *device_path; // Object path of the Bluetooth device the service belongs to
  bool primary;
  characteristic_t * characteristics;
  struct service_t *next;
} service_t;

/**
 * Allocates memory and initialises values for a new service_t 
 * 
 * @param uuid the uuid of the service 
 * @param device_path the path to the device the service belongs to 
 * @param primary true/false if the service is the devices primary service
 * @param characteristics services characteristics 
 * @return initialised service  
 **/
service_t *service_new (const char* uuid, const char *device_path, bool primary, characteristic_t *characteristics);

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
characteristic_t *service_get_characteristic (service_t *service, const char* characteristic_uuid);

/**
 * Adds a characteristic to the service
 * 
 * @param service service to add the characteristic to
 * @param characteristic characteristic to add 
 * @return success true/false
 **/
bool service_add_characteristic (service_t *service, characteristic_t *characteristic);

// TODO
// //DBus methods
// void service_get_all (service_t *service);

#endif //BLE_SIM_SERVICE_H
