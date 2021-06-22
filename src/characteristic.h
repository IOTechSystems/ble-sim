/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_CHARACTERISTIC_H
#define BLE_SIM_CHARACTERISTIC_H

#include <stdint.h>
#include <dbus/dbus.h>

#include "defines.h"
#include "descriptor.h"

typedef struct characteristic_t
{
  char *uuid; //128-bit characteristic UUID.
  char *service_path; //Object path of the GATT service the characteristic belongs to.
  char *object_path; //Object path of the characteristic object
  uint8_t *value; //The characteristic's value
  uint32_t value_size;
  bool notifying; //if notifications or indications on this	characteristic are currently enabled
  uint32_t flags; //Flags to define how the characteristic value can be used
  descriptor_t *descriptors;
  unsigned int descriptor_count;
  struct characteristic_t *next;
} characteristic_t;

/**
 * Allocates memory and initialises values for a new characteristic_t 
 * 
 * @param uuid the uuid of the characteristic 
 * @param descriptors characteristic descriptors 
 * @return initialised characteristic  
 **/
characteristic_t *characteristic_new (const char *uuid);

/**
 * Frees a characteristic_t and it's values
 * 
 * @param characteristic characteristic to free 
 **/
void characteristic_free (characteristic_t *characteristic);

/**
 * Searches the characteristic for a descriptor
 * 
 * @param characteristic characteristic to search 
 * @param descriptor_uuid uuid of the descriptor
 * @return found descriptor or NULL if not found
 **/
descriptor_t *characteristic_get_descriptor (characteristic_t *characteristic, const char *descriptor_uuid);

/**
 * Adds a descriptor to the characteristic
 * 
 * @param characteristic characteristic to add the descriptor to
 * @param descriptor descriptor to add 
 * @return success true/false
 **/
bool characteristic_add_descriptor (characteristic_t *characteristic, descriptor_t *descriptor);

bool characteristic_register (characteristic_t *characteristic);
//DBus Methods TODO
void characteristic_get_object (characteristic_t *characteristic, DBusMessageIter *iter);
// void characteristic_get_all (characteristic_t *characteristic);
// void characteristic_properties_changed (characteristic_t *characteristic);
// //Bluez methods
// void characteristic_read_value (characteristic_t *characteristic);
// void characteristic_write_value (characteristic_t *characteristic);
// void characteristic_aquire_write (characteristic_t *characteristic);
// void characteristic_aquire_notify (characteristic_t *characteristic);
// void characteristic_start_notify (characteristic_t *characteristic);
// void characteristic_stop_notify (characteristic_t *characteristic);

#endif //BLE_SIM_CHARACTERISTIC_H
