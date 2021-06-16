/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_CHARACTERISTIC_H
#define BLE_SIM_CHARACTERISTIC_H

#include <stdint.h>

#include "defines.h"
#include "descriptor.h"

typedef struct characteristic_t
{
  char *UUID; //128-bit characteristic UUID.
  char *service_path; //Object path of the GATT service the characteristic belongs to.
  uint8_t * value;
  uint32_t value_size;
  bool notifying;
  uint32_t flags;
  descriptor_t *descriptors;

  struct characteristic_t *next;
} characteristic_t;

characteristic_t *characteristic_new (const char *UUID, const char *service_path, descriptor_t *descriptors);
void characteristic_free (characteristic_t *characteristic);

descriptor_t *characteristic_get_descriptor (characteristic_t *characteristic, const char *descriptor_uuid);
bool characteristic_add_descriptor (characteristic_t *characteristic, descriptor_t *descriptor);

//DBus Methods
void characteristic_get_all (characteristic_t *characteristic);
void characteristic_properties_changed (characteristic_t *characteristic);
//Bluez methods
void characteristic_read_value (characteristic_t *characteristic);
void characteristic_write_value (characteristic_t *characteristic);
void characteristic_aquire_write (characteristic_t *characteristic);
void characteristic_aquire_notify (characteristic_t *characteristic);
void characteristic_start_notify (characteristic_t *characteristic);
void characteristic_stop_notify (characteristic_t *characteristic);

#endif //BLE_SIM_CHARACTERISTIC_H
