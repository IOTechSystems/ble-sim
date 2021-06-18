/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_DESCRIPTOR_H
#define BLE_SIM_DESCRIPTOR_H

#include <stdint.h>
#include <dbus/dbus.h>

typedef struct descriptor_t
{
  char *uuid; //128-bit descriptor UUID.
  char *characteristic_path; //Object path of the GATT characteristic the descriptor belongs to.
  char *object_path;
  uint8_t *value;
  uint32_t value_size;
  uint16_t flags;
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
descriptor_t *descriptor_new (const char *uuid, const char *characteristic_path);

/**
 * Frees a descriptor_t and it's values
 * 
 * @param descriptor descriptor to free 
 **/
void descriptor_free (descriptor_t *descriptor);

// TODO
// //DBus methods
// void descriptor_get_all (descriptor_t *descriptor);

// //Bluez methods
// void descriptor_read_value (descriptor_t *descriptor);
// void descriptor_write_value (descriptor_t *descriptor);

#endif //BLE_SIM_DESCRIPTOR_H
