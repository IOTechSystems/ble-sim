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
  char *UUID; //128-bit service UUID
  char *device_path; // Object path of the Bluetooth device the service belongs to
  bool primary;
  characteristic_t * characteristics;
  
  struct service_t *next;
} service_t;

service_t *service_new(const char* UUID, const char *device_path, bool primary, characteristic_t *characteristics);
void service_free(service_t *service);

characteristic_t *service_get_characteristic(service_t *service, const char* characteristic_uuid);
bool service_add_characteristic(service_t *service, characteristic_t *characteristic);


//DBus methods
void service_get_all(service_t *service);

#endif //BLE_SIM_SERVICE_H