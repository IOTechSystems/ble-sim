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

static void service_handle_unregister_device (DBusConnection *connection, void *data);
static DBusHandlerResult service_handle_dbus_message (DBusConnection *connection, DBusMessage *message, void *data);
static void service_get_uuid (void *user_data, DBusMessageIter* iter);
static void service_get_device_path (void *user_data, DBusMessageIter* iter);
static void service_get_primary (void *user_data, DBusMessageIter* iter);

DBusObjectPathVTable service_dbus_callbacks = {
  .unregister_function = service_handle_unregister_device,
  .message_function = service_handle_dbus_message,
};

static dbus_property_t service_properties[] =
{
  {BLE_PROPERTY_UUID, DBUS_TYPE_STRING_AS_STRING, service_get_uuid},
  {BLE_PROPERTY_DEVICE, DBUS_TYPE_OBJECT_PATH_AS_STRING, service_get_device_path},
  {BLE_PROPERTY_PRIMARY, DBUS_TYPE_BOOLEAN_AS_STRING, service_get_primary},
  DBUS_PROPERTY_NULL
};

service_t *service_new (const char* uuid, bool primary, characteristic_t *characteristics)
{
  service_t *new_service = calloc (1, sizeof (*new_service));
  if (NULL == new_service)
  {
    return NULL;
  }

  new_service->uuid = strdup (uuid);
  new_service->device_path = NULL;
  new_service->object_path = NULL;
  new_service->primary = primary;
  new_service->characteristics = characteristics;
  new_service->characteristic_count = 0;
  new_service->next = NULL;

  return new_service;
}

void service_free (service_t *service)
{
  if (NULL == service)
  {
    return;
  }

  free (service->uuid);
  free (service->device_path);
  free (service->object_path);

  characteristic_t *tmp = NULL;
  while (service->characteristics)
  {
    tmp = service->characteristics->next;
    characteristic_free (service->characteristics);
    service->characteristics = tmp;
  }
  service->next = NULL;

  free(service);
}

//device object dbus functions
static void service_handle_unregister_device (DBusConnection *connection, void *data)
{

}

static DBusHandlerResult service_handle_dbus_message (DBusConnection *connection, DBusMessage *message, void *data)
{
  service_t *service = (service_t *) data;
  printf ("SERVICE MESSAGE: got dbus message sent to %s %s %s (service: %s) \n",
          dbus_message_get_destination(message),
          dbus_message_get_interface(message),
          dbus_message_get_path(message),
          service->uuid
          );

  return DBUS_HANDLER_RESULT_HANDLED;
}

characteristic_t *service_get_characteristic (service_t *service, const char* characteristic_uuid)
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
  if (service_get_characteristic (service, characteristic->uuid))
  {
    return false;
  }

  char *characteristic_object_path = dbusutils_create_object_path (service->object_path, CHARACTERISTIC_OBJECT_NAME, service->characteristic_count);
  if (!dbusutils_register_object (global_dbus_connection, characteristic_object_path, &characteristic_dbus_callbacks, characteristic))
  {
    free (characteristic_object_path);
    return false;
  }
  characteristic->object_path = characteristic_object_path;
  characteristic->service_path = strdup (service->object_path);

  characteristic->next = service->characteristics;
  service->characteristics = characteristic;
  service->characteristic_count++;

  return true;
}

//DBUS
static void service_get_uuid (void *user_data, DBusMessageIter* iter)
{
  service_t* service = (service_t*) user_data;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, &service->uuid);
}

static void service_get_device_path (void *user_data, DBusMessageIter* iter)
{
  service_t* service = (service_t*) user_data;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_OBJECT_PATH, &service->device_path);
}

static void service_get_primary (void *user_data, DBusMessageIter* iter)
{
  service_t* service = (service_t*) user_data;
  dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &service->primary);
}

void service_get_object(service_t *service, DBusMessageIter* iter)
{
  dbusutils_get_object_data (iter, &service_properties[0], service->object_path, BLUEZ_GATT_SERVICE_INTERFACE, service);
}

