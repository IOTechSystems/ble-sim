/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <stdlib.h>
#include <string.h>

#include "descriptor.h"

descriptor_t *descriptor_new(const char *UUID, const char *characteristic_path)
{
  descriptor_t *new_descriptor = calloc(1, sizeof(*new_descriptor));
  if (NULL == new_descriptor)
  {
    return NULL;
  }

  new_descriptor->UUID = strdup(UUID);
  new_descriptor->characteristic_path = strdup(characteristic_path);
  new_descriptor->value = NULL;
  new_descriptor->value_size = 0;
  new_descriptor->flags = 0xFFFF;
  new_descriptor->next = NULL;

  return new_descriptor;
}

void descriptor_free(descriptor_t *descriptor)
{
  if (NULL == descriptor)
  {
    return;
  }

  free(descriptor->UUID);
  free(descriptor->characteristic_path);
  free(descriptor->value);
  free(descriptor);
}

//DBus methods
void descriptor_get_all(descriptor_t *descriptor)
{

}

//Bluez methods
void descriptor_read_value(descriptor_t *descriptor)
{

}

void descriptor_write_value(descriptor_t *descriptor)
{

}