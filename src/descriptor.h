/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_DESCRIPTOR_H
#define BLE_SIM_DESCRIPTOR_H

#include <stdint.h>

typedef struct descriptor_t
{
  char *UUID; //128-bit descriptor UUID.
  char *characteristic_path; //Object path of the GATT characteristic the descriptor belongs to.
  uint8_t * value;
  uint32_t value_size;
  uint16_t flags;

  struct descriptor_t *next;
} descriptor_t;

descriptor_t *descriptor_new (const char *UUID, const char *characteristic_path);
void descriptor_free (descriptor_t *descriptor);

//DBus methods
void descriptor_get_all (descriptor_t *descriptor);

//Bluez methods
void descriptor_read_value (descriptor_t *descriptor);
void descriptor_write_value (descriptor_t *descriptor);

#endif //BLE_SIM_DESCRIPTOR_H
