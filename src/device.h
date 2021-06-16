/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_DEVICE_H
#define BLE_SIM_DEVICE_H

#include <stdlib.h>

#include "defines.h"
#include "server.h"


typedef struct device_t
{
  server_t *gatt_server;
  char *controller; //does nothing for now - in future this will be the controller this "device" is exposed on
  char *device_name;
  struct device_t *next;

} device_t;

device_t * device_get_device(const char* device_name);
bool device_add(const char* device_name);
bool device_add_service (const char* device_name, service_t *service);
bool device_add_characteristic (const char* device_name, const char *service_uuid, characteristic_t *characteristic);
bool device_add_descriptor (const char* device_name, const char *service_uuid, const char *characteristic_uuid, descriptor_t *descriptor);

void device_cleanup_devices(void);

#endif //BLE_SIM_DEVICE_H