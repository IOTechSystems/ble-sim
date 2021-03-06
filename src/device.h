/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_DEVICE_H
#define BLE_SIM_DEVICE_H

#include <stdlib.h>
#include <stdbool.h>

#include <dbus/dbus.h>

#include "bluez/emulator/vhci.h"

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
  bool initialised; //if device has been sucessfully registered and initialised and is operation
  int origin;//where the object was created - influences how we free it
  struct vhci *virtual_controller;
  advertisement_t advertisement; //advertisement
  struct device_t *next;
} device_t;

/**
 * Initialises values for a new device_t 
 * @param device device to initialise
 * @param device_name name of the device
 * @param the origin of the object used to distinguish if it was created in lua
 * @return initialised device  
 **/
void device_init (device_t *device, const char *device_name, int origin);

/**
 * Frees an initialised devices values
 *  @param device device
 * */
void device_fini (device_t *device);

/**
 * Frees a device and it's data 
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
 * initialises a devices controller,
 * registers the device as a dbus object 
 * registers it with bluez and then
 * adds the device to the internal device list,
 * 
 * @param device the device to add
 * @return successful true/false
 **/
bool device_register (device_t *device);

/**
 * Removes a device from the list
 * @param device_name unique name of the device
 * @return successful true/false if the device was added to the list
 **/
bool device_remove (const char *device_name);

/**
 * Adds a service to device
 * @param device_name unique name of the device to add the service to
 * @param service service to add
 * @return successful true/false
 **/
bool device_add_service (device_t *device, service_t *service);

/**
 * Sets a devices discoverabilty 
 * @param device the device
 * @param discoverable true/false
 * @return success true/false 
 **/
bool device_set_discoverable (device_t *device, bool discoverable);

/**
 * Powers on/off a device 
 * @param device the device
 * @param powered true/false
 * @return success true/false 
 **/
bool device_set_powered (device_t *device, bool powered);

#endif //BLE_SIM_DEVICE_H
