/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <string.h>

#include "device.h"
#include "service.h"
#include "characteristic.h"
#include "descriptor.h"

static device_t *device_list_head = NULL;

static device_t *device_new (const char* device_name, const char* controller)
{
  device_t *device = calloc (1, sizeof (*device));
  if (NULL == device)
  {
    return NULL;
  }

  device->gatt_server = server_new ("", NULL);
  device->device_name = strdup (device_name);
  device->controller = strdup (controller);
  device->next = NULL;

  return device;
}

static void device_free (device_t *device)
{
  if (NULL == device)
  {
    return;
  }

  free (device->controller);
  free (device->device_name);
  server_free (device->gatt_server);
  free (device);
}

bool device_add (const char* device_name)
{
  if (device_get_device (device_name))
  {
    printf ("Device with that name already exists\n");
    return false;
  }

  device_t *new_device = device_new (device_name, "CONTROLLER");

  //init controller for device
  if (device_list_head == NULL)
  {
    device_list_head = new_device;
  }

  return true;
}

void device_cleanup_devices (void)
{
  device_t *tmp = NULL;
  while (device_list_head)
  {
    tmp = device_list_head->next;
    device_free (device_list_head);
    device_list_head = tmp;
  }
}

device_t * device_get_device (const char* device_name)
{
  device_t *device = device_list_head;
  while (device)
  {
    if (strcmp (device_name, device->device_name) == 0)
    {
      return device;
    }
    device = device->next;
  }
  return NULL;
}

bool device_add_service (const char* device_name, service_t *service)
{
  device_t *device = device_get_device (device_name);
  if (NULL == device)
  {
    printf ("Could not add service to device %s, as the device doesn't exist", device_name);
    return false;
  }

  return server_add_service (device->gatt_server, service);
}

bool device_add_characteristic (const char* device_name, 
                                const char *service_uuid, 
                                characteristic_t *characteristic)
{
  device_t *device = device_get_device (device_name);
  if (NULL == device)
  {
    printf ("Could not add characteristic to device %s as the device doesn't exist", device_name);
    return false;
  }

  service_t *service = server_get_service (device->gatt_server, service_uuid);
  if (NULL == service)
  {
    printf ("Could not add characteristic to device %s as the service with uuid %s doesn't exist", device_name, service_uuid);
    return false;
  }

  return service_add_characteristic (service, characteristic);
}

bool device_add_descriptor (const char* device_name, 
                            const char *service_uuid, 
                            const char *characteristic_uuid, 
                            descriptor_t *descriptor)
{
  device_t *device = device_get_device (device_name);
  if (NULL == device)
  {
    printf ("Could not add descriptor to device %s as the device doesn't exist", device_name);
    return false;
  }

  service_t *service = server_get_service (device->gatt_server, service_uuid);
  if (NULL == service)
  {
    printf ("Could not add descriptor to device %s as the service with uuid %s doesn't exist", device_name, service_uuid);
    return false;
  }

  characteristic_t *characteristic = service_get_characteristic (service, characteristic_uuid);
  if (NULL == characteristic)
  {
    printf ("Could not add characteristic to device %s as the characteristic with uuid %s doesn't exist", device_name, characteristic_uuid);
    return false;
  }

  return characteristic_add_descriptor (characteristic, descriptor);
}
