/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_DEVICE_H
#define BLE_SIM_DEVICE_H

#include <stdlib.h>

#include <dbus/dbus.h>

#include "defines.h"
#include "service.h"
#include "advertising.h"

extern DBusConnection *global_dbus_connection;

typedef struct device_t
{
  service_t *services; //list of services
  unsigned int service_count;
  char *controller; //path to bluez controller
  char *device_name; //name of the device
  char *object_path; //dbus object path to register to
  bool application_registered;
  advertisement_t advertisement; //advertisement
  struct device_t *next;
} device_t;

/**
 * Allocates memory and initialises values for a new device_t 
 * 
 * @param device_name name of the device
 * @param controller name of the controller 
 * @return initialised device  
 **/
device_t *device_new (const char *device_name, const char *controller);

/**
 * Frees a device and it's values
 * 
 * @param device device to free 
 **/
void device_free (device_t *device);

/**
 * Searches internal device list for a device with a matching device_name
 *
 * @param device_name name of the device to find
 * @return found device or NULL if not found  
 **/
device_t *device_get_device (const char *device_name);

/**
 * Adds a device to the internal device list, registers it as a dbus object 
 * and attempts to register it with bluez
 * 
 * @param device the device to add
 * @return succesful true/false
 **/
bool device_add (device_t *device);

/**
 * Removes a device from the list
 * @param device_name unique name of the device
 * @return succesful true/false if the device was added to the list
 **/
bool device_remove (const char *device_name);

/**
 * Adds a service to device
 * @param device_name unique name of the device to add the service to
 * @param service service to add
 * @return succesful true/false
 **/
bool device_add_service (device_t *device, service_t *service);

/**
 * Adds a service characteristic to device
 * @param device_name unique name of the device to add the characteristic to
 * @param service_uuid service to add the characteristic to
 * @param characteristic characteristic to add
 * @return succesful true/false
 **/
bool device_add_characteristic (device_t *device, const char *service_uuid, characteristic_t *characteristic);

/**
 * Adds a characteristic descriptor to device
 * @param device_name unique name of the device to add the descriptor to
 * @param service_uuid service to add the descriptor to
 * @param characteristic_uuid characteristic to add the descriptor to
 * @param descriptor descriptor to add
 * @return succesful true/false
 **/
bool device_add_descriptor (device_t *device, const char *service_uuid, const char *characteristic_uuid, descriptor_t *descriptor);

/**
 * Sets a devices discoverabilty 
 * @param device the device
 * @param discoverable true/false
 * @return success true/false 
 **/
bool device_set_discoverable (device_t *device, bool discoverable);

/**
 * Sets devices adverising state 
 * @param device the device
 * @param advertising true/false
 * @return success true/false 
 **/
bool device_set_advertising (device_t *device, bool advertising);

/**
 * Powers on/off a device 
 * @param device the device
 * @param powered true/false
 * @return success true/false 
 **/
bool device_set_powered(device_t *device, bool powered);

/**
 * Removes and frees all devices in the internal device list
 **/
void device_cleanup_devices (void);

#endif //BLE_SIM_DEVICE_H
