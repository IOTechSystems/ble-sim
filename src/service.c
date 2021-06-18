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

static void service_handle_unregister_device(DBusConnection *connection, void *data);
static DBusHandlerResult service_handle_dbus_message(DBusConnection *connection, DBusMessage *message, void *data);
static void service_get_uuid (service_t *service, DBusMessageIter* iter);
static void service_get_device_path (service_t *service, DBusMessageIter* iter);
static void service_get_primary (service_t *service, DBusMessageIter* iter);
static void service_get_properties (service_t *service, DBusMessageIter* iter);

DBusObjectPathVTable service_dbus_callbacks = {
  .unregister_function = service_handle_unregister_device,
  .message_function = service_handle_dbus_message,
};

service_t *service_new (const char* uuid, const char *device_path, bool primary, characteristic_t *characteristics)
{
  service_t *new_service = calloc (1, sizeof(*new_service));
  if (NULL == new_service)
  {
    return NULL;
  }

  new_service->uuid = strdup (uuid);
  new_service->device_path = strdup (device_path);
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
static void service_handle_unregister_device(DBusConnection *connection, void *data)
{

}

static DBusHandlerResult service_handle_dbus_message(DBusConnection *connection, DBusMessage *message, void *data)
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

  characteristic->next = service->characteristics;
  service->characteristics = characteristic;
  service->characteristic_count++;

  return true;
}

//DBUS
static void service_get_uuid (service_t *service, DBusMessageIter* iter)
{
  dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &service->uuid);
}

static void service_get_device_path (service_t *service, DBusMessageIter* iter)
{
  dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &service->device_path);
}

static void service_get_primary (service_t *service, DBusMessageIter* iter)
{
  dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &service->primary);
}

static void service_get_properties (service_t *service, DBusMessageIter* iter)
{
  DBusMessageIter array;
  dbus_message_iter_open_container(
    iter,
    DBUS_TYPE_ARRAY, 
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING //signature "{sv}"
    DBUS_TYPE_STRING_AS_STRING
    DBUS_TYPE_VARIANT_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
    &array
  );

  service_get_uuid (service, &array);
  service_get_device_path (service, &array);
  service_get_primary (service, &array);

  dbus_message_iter_close_container(iter, &array);
}

void service_get_object(service_t *service, DBusMessageIter* iter)
{
  //this might need to be /org/blesim + service->object_path
  dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, service->object_path);

  DBusMessageIter array;
  dbus_message_iter_open_container(
    iter,
    DBUS_TYPE_ARRAY, 
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING //signature "{sa{sv}}"
    DBUS_TYPE_STRING_AS_STRING
    DBUS_TYPE_ARRAY_AS_STRING
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_STRING_AS_STRING
    DBUS_TYPE_VARIANT_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
    &array
  );
  
  dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, BLUEZ_GATT_SERVICE_INTERFACE);
  service_get_properties(service, &array);
  
  dbus_message_iter_close_container(iter, &array);
}

