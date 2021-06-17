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

/**
 * Searches internal device list for a device with a matching device_name
 *
 * @param device_name name of the device to find
 * @return found device or NULL if not found  
 **/
device_t * device_get_device (const char* device_name);

/**
 * Creates a new device and adds it to the internal device list
 * @param device_name unique name of the device
 * @return succesful true/false
 **/
bool device_add (const char* device_name);

/**
 * Adds a service to specified device
 * @param device_name unique name of the device to add the service to
 * @param service service to add
 * @return succesful true/false
 **/
bool device_add_service (const char* device_name, service_t *service);

/**
 * Adds a service characteristic to specified device
 * @param device_name unique name of the device to add the characteristic to
 * @param service_uuid service to add the characteristic to
 * @param characteristic characteristic to add
 * @return succesful true/false
 **/
bool device_add_characteristic (const char* device_name, const char *service_uuid, characteristic_t *characteristic);

/**
 * Adds a characteristic descriptor to specified device
 * @param device_name unique name of the device to add the descriptor to
 * @param service_uuid service to add the descriptor to
 * @param characteristic_uuid characteristic to add the descriptor to
 * @param descriptor descriptor to add
 * @return succesful true/false
 **/
bool device_add_descriptor (const char* device_name, const char *service_uuid, const char *characteristic_uuid, descriptor_t *descriptor);

/**
 * Removes and frees all devices in the internal device list
 **/
void device_cleanup_devices (void);

#endif //BLE_SIM_DEVICE_H
