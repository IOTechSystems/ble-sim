/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "characteristic.h"

characteristic_t *characteristic_new (const char *UUID, const char *service_path, descriptor_t *descriptors)
{
  characteristic_t *new_characteristic = calloc (1, sizeof (*new_characteristic));
  if (NULL == new_characteristic)
  {
    return NULL;
  }

  new_characteristic->UUID = strdup (UUID);
  new_characteristic->service_path = strdup (service_path);
  new_characteristic->value = NULL;
  new_characteristic->value_size = 0;
  new_characteristic->flags = 0xFFFFFFFF; //all enabled for now
  new_characteristic->descriptors = descriptors;

  new_characteristic->next = NULL;

  return new_characteristic;
}

void characteristic_free (characteristic_t *characteristic)
{
  if (NULL == characteristic)
  {
    return;
  }

  free (characteristic->UUID);
  free (characteristic->service_path);
  free (characteristic->value);

  descriptor_t *tmp = NULL;
  while (characteristic->descriptors)
  {
    tmp = characteristic->descriptors->next;
    descriptor_free (characteristic->descriptors);
    characteristic->descriptors = tmp;
  }

  free (characteristic);
}

descriptor_t *characteristic_get_descriptor (characteristic_t *characteristic, const char *descriptor_uuid)
{
  descriptor_t *descriptor = characteristic->descriptors;
  while (descriptor)
  {
    if (strcmp(descriptor_uuid, descriptor->UUID) == 0)
    {
      return descriptor;
    }
    descriptor = descriptor->next;
  }

  return NULL; 
}

bool characteristic_add_descriptor (characteristic_t *characteristic, descriptor_t *descriptor)
{
  if (characteristic_get_descriptor (characteristic, descriptor->UUID))
  {
    return false;
  }

  descriptor->next = characteristic->descriptors;
  characteristic->descriptors = descriptor;
  return true;
}

//DBus Methods
void characteristic_get_all (characteristic_t *characteristic)
{

}

void characteristic_properties_changed (characteristic_t *characteristic)
{

}

//Bluez methods
void characteristic_read_value (characteristic_t *characteristic)
{

}

void characteristic_write_value (characteristic_t *characteristic)
{

}

void characteristic_aquire_write (characteristic_t *characteristic)
{

}

void characteristic_aquire_notify (characteristic_t *characteristic)
{

}

void characteristic_start_notify (characteristic_t *characteristic)
{

}

void characteristic_stop_notify (characteristic_t *characteristic)
{

}
