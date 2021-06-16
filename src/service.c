/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/
#include <stdio.h>
#include <string.h>

#include "service.h"

service_t *service_new(const char* UUID, const char *device_path, bool primary, characteristic_t *characteristics)
{
  service_t *new_service = calloc(1, sizeof(*new_service));
  if (NULL == new_service)
  {
    return NULL;
  }

  new_service->UUID = strdup(UUID);
  new_service->device_path = strdup(device_path);
  new_service->primary = primary;
  new_service->characteristics = characteristics;

  new_service->next = NULL;

  return new_service;
}

void service_free(service_t *service)
{
  if (NULL == service)
  {
    return;
  }

  free(service->UUID);
  free(service->device_path);

  characteristic_t *tmp = NULL;
  while(service->characteristics)
  {
    tmp = service->characteristics->next;
    characteristic_free(service->characteristics);
    service->characteristics = tmp;
  }

  service->next = NULL;

  free(service);
}

void service_get_all(service_t *service)
{
  return;
}

characteristic_t *service_get_characteristic (service_t *service, const char* characteristic_uuid)
{
  characteristic_t *characteristic = service->characteristics;
  while (characteristic)
  {
    if (strcmp (characteristic_uuid, characteristic->UUID) == 0)
    {
      return characteristic;
    }
    characteristic = characteristic->next;
  }

  return NULL; 
}

bool service_add_characteristic(service_t *service, characteristic_t *characteristic)
{
  if (service_get_characteristic(service, characteristic->UUID))
  {
    return false;
  }

  characteristic->next = service->characteristics;
  service->characteristics = characteristic;
  return true;
}