/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_DESCRIPTOR_H
#define BLE_SIM_DESCRIPTOR_H

#include <stdint.h>
#include <stdbool.h>

#include <dbus/dbus.h>

typedef struct descriptor_t
{
  char *uuid; //128-bit descriptor UUID.
  char *characteristic_path; //Object path of the GATT characteristic the descriptor belongs to.
  char *object_path; //Object path of the descriptor object
  uint8_t *value; //Descriptors value
  uint32_t value_size;
  uint16_t flags; //Flags that define how the descriptor value can be used
  struct descriptor_t *next;
} descriptor_t;

extern DBusObjectPathVTable descriptor_dbus_callbacks;

/**
 * Allocates memory and initialises values for a new characteristic_t 
 * 
 * @param uuid the uuid of the descriptor
 * @param characteristic the path to the characteristic the descriptor belongs to 
 * @return initialised descriptor  
 **/
descriptor_t *descriptor_new (const char *uuid);

/**
 * Frees a descriptor_t and it's values
 * 
 * @param descriptor descriptor to free 
 **/
void descriptor_free (descriptor_t *descriptor);

/**
 * Registers the descriptor object with dbus
 * @param descriptor pointer to the descriptor
 * @return success true/false
 **/
bool descriptor_register (descriptor_t *descriptor);

//DBus methods
/**
 * Populates a dbus message iter with a descriptor's object data
 * @param descriptor pointer to the descriptor
 * @param iter dbus message iter to populate
 * @return 
 **/
void descriptor_get_object (descriptor_t *descriptor, DBusMessageIter *iter);

// //Bluez methods
// void descriptor_read_value (descriptor_t *descriptor);
// void descriptor_write_value (descriptor_t *descriptor);

#endif //BLE_SIM_DESCRIPTOR_H
