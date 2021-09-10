/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/
#include <stdio.h>
#include <string.h>

#include "service.h"
#include "characteristic.h"
#include "dbusutils.h"
#include "logger.h"


static void service_get_uuid (void *user_data, DBusMessageIter *iter);

static void service_get_device_path (void *user_data, DBusMessageIter *iter);

static void service_get_primary (void *user_data, DBusMessageIter *iter);

static dbus_property_t service_properties[] =
  {
    {BLE_PROPERTY_UUID, DBUS_TYPE_STRING_AS_STRING, service_get_uuid},
    {BLE_PROPERTY_DEVICE, DBUS_TYPE_OBJECT_PATH_AS_STRING, service_get_device_path},
    {BLE_PROPERTY_PRIMARY, DBUS_TYPE_BOOLEAN_AS_STRING, service_get_primary},
    DBUS_PROPERTY_NULL
  };

static dbus_method_t service_methods[] =
  {
    DBUS_METHOD_NULL
  };

service_t *service_init (service_t *service, const char *uuid, bool primary, int origin)
{
  service->origin = origin;
  service->uuid = strdup (uuid);
  service->device_path = NULL;
  service->object_path = NULL;
  service->primary = primary;
  service->characteristics = NULL;
  service->characteristic_count = 0;
  service->next = NULL;

  return service;
}

void service_fini (service_t *service)
{
  if (NULL == service)
  {
    return;
  }

  free (service->uuid);
  free (service->device_path);
  free (service->object_path);

  if (service->origin == ORIGIN_C)
  {
    characteristic_t *tmp = NULL;
    while (service->characteristics)
    {
      tmp = service->characteristics->next;
      characteristic_free (service->characteristics);
      service->characteristics = tmp;
    }
    service->next = NULL;
  }

}

void service_free (service_t *service)
{
  service_fini (service);
  free (service);
}

characteristic_t *service_get_characteristic (service_t *service, const char *characteristic_uuid)
{
  characteristic_t *characteristic = service->characteristics;
  while (characteristic)
  {
    if (strcmp (characteristic_uuid, characteristic->uuid) == 0)
    {
      return characteristic;
    }
    characteristic = characteristic->next;
  }

  return NULL;
}

bool service_add_characteristic (service_t *service, characteristic_t *characteristic)
{
  if (NULL == service->object_path)
  {
    log_warn ("Service must be added to a device first in order to add a characteristic to it!");
    return false;
  }

  if (NULL != characteristic->service_path)
  {
    log_warn ("Characteristic already belongs to another service.");
    return false;
  }

  if (service_get_characteristic (service, characteristic->uuid))
  {
    return false;
  }

  characteristic->object_path = dbusutils_create_object_path (service->object_path, CHARACTERISTIC_OBJECT_NAME, service->characteristic_count);
  if (!characteristic_register (characteristic))
  {
    free (characteristic->object_path);
    return false;
  }
  characteristic->service_path = strdup (service->object_path);

  characteristic->next = service->characteristics;
  service->characteristics = characteristic;
  service->characteristic_count++;

  return true;
}

bool service_register (service_t *service)
{
  return dbusutils_register_object (global_dbus_connection, service->object_path, service_properties, service_methods, service);
}

//DBUS
static void service_get_uuid (void *user_data, DBusMessageIter *iter)
{
  service_t *service = (service_t *) user_data;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, &service->uuid);
}

static void service_get_device_path (void *user_data, DBusMessageIter *iter)
{
  service_t *service = (service_t *) user_data;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_OBJECT_PATH, &service->device_path);
}

static void service_get_primary (void *user_data, DBusMessageIter *iter)
{
  service_t *service = (service_t *) user_data;
  dbus_bool_t primary = service->primary ? TRUE : FALSE;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_BOOLEAN, &primary);
}

void service_get_object (service_t *service, DBusMessageIter *iter)
{
  dbusutils_get_object_data (iter, service_properties, service->object_path, BLUEZ_GATT_SERVICE_INTERFACE, service);
}

