/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "characteristic.h"
#include "descriptor.h"
#include "dbusutils.h"
#include "defines.h"
#include "utils.h"

static void characteristic_set_value (characteristic_t *characteristic, const void *new_value, const uint32_t value_size);

static void characteristic_get_uuid (void *user_data, DBusMessageIter *iter);

static void characteristic_get_service (void *user_data, DBusMessageIter *iter);

static void characteristic_get_flags (void *user_data, DBusMessageIter *iter);

static void characteristic_get_value (void *user_data, DBusMessageIter *iter);

static DBusMessage *characteristic_read_value (void *user_data, DBusConnection *connection, DBusMessage *message);

static DBusMessage *characteristic_write_value (void *user_data, DBusConnection *connection, DBusMessage *message);

static DBusMessage *characteristic_start_notify (void *user_data, DBusConnection *connection, DBusMessage *message);

static DBusMessage *characteristic_stop_notify (void *user_data, DBusConnection *connection, DBusMessage *message);

static dbus_property_t characteristic_properties[] =
  {
    {BLE_PROPERTY_UUID, DBUS_TYPE_STRING_AS_STRING, characteristic_get_uuid},
    {BLE_PROPERTY_SERVICE, DBUS_TYPE_OBJECT_PATH_AS_STRING, characteristic_get_service},
    {BLE_PROPERTY_FLAGS, DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING, characteristic_get_flags},
    {BLE_PROPERTY_VALUE, DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_BYTE_AS_STRING, characteristic_get_value},
    DBUS_PROPERTY_NULL
  };

static dbus_method_t characteristic_methods[] =
  {
    {BLUEZ_GATT_CHARACTERISTIC_INTERFACE, BLUEZ_METHOD_READ_VALUE, characteristic_read_value},
    {BLUEZ_GATT_CHARACTERISTIC_INTERFACE, BLUEZ_METHOD_WRITE_VALUE, characteristic_write_value},
    {BLUEZ_GATT_CHARACTERISTIC_INTERFACE, BLUEZ_METHOD_START_NOTIFY, characteristic_start_notify},
    {BLUEZ_GATT_CHARACTERISTIC_INTERFACE, BLUEZ_METHOD_STOP_NOTIFY, characteristic_stop_notify},
    DBUS_METHOD_NULL
  };

static object_flag_t characteristic_flags[] =
  {
    {CHARACTERISTIC_FLAG_BROADCAST,                     CHARACTERISTIC_FLAG_BROADCAST_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_READ,                          CHARACTERISTIC_FLAG_READ_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_WRITE_WITHOUT_RESPONSE,        CHARACTERISTIC_FLAG_WRITE_WITHOUT_RESPONSE_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_WRITE,                         CHARACTERISTIC_FLAG_WRITE_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_NOTIFY,                        CHARACTERISTIC_FLAG_NOTIFY_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_INDICATE,                      CHARACTERISTIC_FLAG_INDICATE_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_AUTHENTICATED_SIGNED_WRITES,   CHARACTERISTIC_FLAG_AUTHENTICATED_SIGNED_WRITES_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_EXTENDED_PROPERTIES,           CHARACTERISTIC_FLAG_EXTENDED_PROPERTIES_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_RELIABLE_WRITE,                CHARACTERISTIC_FLAG_RELIABLE_WRITE_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_WRITABLE_AUXILIARIES,          CHARACTERISTIC_FLAG_WRITABLE_AUXILIARIES_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_ENCRYPTED_READ,                CHARACTERISTIC_FLAG_ENCRYPTED_READ_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_ENCRYPTED_WRITE,               CHARACTERISTIC_FLAG_ENCRYPTED_WRITE_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_ENCRYPTED_AUTHENTICATED_READ,  CHARACTERISTIC_FLAG_ENCRYPTED_AUTHENTICATED_READ_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_ENCRYPTED_AUTHENTICATED_WRITE, CHARACTERISTIC_FLAG_ENCRYPTED_AUTHENTICATED_WRITE_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_SECURE_READ,                   CHARACTERISTIC_FLAG_SECURE_READ_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_SECURE_WRITE,                  CHARACTERISTIC_FLAG_SECURE_WRITE_ENABLED_BIT},
    {CHARACTERISTIC_FLAG_AUTHORIZE,                     CHARACTERISTIC_FLAG_AUTHORIZE_ENABLED_BIT}
  };

characteristic_t *characteristic_new (void)
{
  characteristic_t *new_characteristic = calloc (1, sizeof (*new_characteristic));
  if (NULL == new_characteristic)
  {
    return NULL;
  }

  return new_characteristic;
}

characteristic_t *characteristic_init (characteristic_t *characteristic, const char *uuid, int origin)
{
  characteristic->origin = origin;
  characteristic->uuid = strdup (uuid);
  characteristic->service_path = NULL;
  characteristic->object_path = NULL;

  characteristic->value = NULL;
  characteristic->value_size = 0;

  characteristic->notifying = false;
  characteristic->flags = CHARACTERISTIC_FLAGS_ALL_ENABLED; //all enabled for now
  characteristic->descriptors = NULL;
  characteristic->descriptor_count = 0;
  characteristic->next = NULL;
  return characteristic;
}

void characteristic_free (characteristic_t *characteristic)
{
  if (NULL == characteristic)
  {
    return;
  }

  free (characteristic->uuid);
  free (characteristic->service_path);
  free (characteristic->object_path);
  free (characteristic->value);

  if (characteristic->origin == ORIGIN_C)
  {
    descriptor_t *tmp = NULL;
    while (characteristic->descriptors)
    {
      tmp = characteristic->descriptors->next;
      descriptor_free (characteristic->descriptors);
      characteristic->descriptors = tmp;
    }

    free (characteristic);
  }
}

descriptor_t *characteristic_get_descriptor (characteristic_t *characteristic, const char *descriptor_uuid)
{
  descriptor_t *descriptor = characteristic->descriptors;
  while (descriptor)
  {
    if (strcmp (descriptor_uuid, descriptor->uuid) == 0)
    {
      return descriptor;
    }
    descriptor = descriptor->next;
  }

  return NULL;
}

bool characteristic_add_descriptor (characteristic_t *characteristic, descriptor_t *descriptor)
{
  if (NULL == characteristic->object_path)
  {
    printf ("ERR: Characteristic must be added to a service first in order to add a descriptor to it.\n");
    return false;
  }

  if (NULL != descriptor->characteristic_path)
  {
    printf ("ERR: Descriptor already belongs to another characteristic.\n");
    return false;
  }

  if (characteristic_get_descriptor (characteristic, descriptor->uuid))
  {
    return false;
  }

  descriptor->object_path = dbusutils_create_object_path (characteristic->object_path, DESCRIPTOR_OBJECT_NAME, characteristic->descriptor_count);
  if (!descriptor_register (descriptor))
  {
    free (descriptor->object_path);
    return false;
  }
  descriptor->characteristic_path = strdup (characteristic->object_path);

  descriptor->next = characteristic->descriptors;
  characteristic->descriptors = descriptor;
  characteristic->descriptor_count++;
  return true;
}

bool characteristic_register (characteristic_t *characteristic)
{
  return dbusutils_register_object (global_dbus_connection, characteristic->object_path, characteristic_properties, characteristic_methods, characteristic);
}

static bool is_new_value (characteristic_t *characteristic, const void *new_value, const uint32_t value_size)
{
  if (characteristic->value_size != value_size)
  {
    return true;
  }

  if (memcmp (new_value, characteristic->value, value_size) != 0)
  {
    return true;
  }

  return false;
}

void characteristic_update_value (characteristic_t *characteristic, const void *new_value, const uint32_t value_size, DBusConnection *connection)
{
  if (!is_new_value (characteristic, new_value, value_size))
  {
    return; //no point in updating if the value is the same so return
  }

  characteristic_set_value (characteristic, new_value, value_size);

  if (characteristic->notifying)
  {
    dbus_property_t changed_property[] = {
      {BLE_PROPERTY_VALUE, DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_BYTE_AS_STRING, characteristic_get_value},
      DBUS_PROPERTY_NULL
    };
    dbusutils_send_object_properties_changed_signal (connection, characteristic->object_path, BLUEZ_GATT_CHARACTERISTIC_INTERFACE, changed_property,
                                                     characteristic);
  }
}

void characteristic_set_notifying (characteristic_t *characteristic, bool notifying)
{
  characteristic->notifying = notifying;
}

//DBus Methods
void characteristic_get_object (characteristic_t *characteristic, DBusMessageIter *iter)
{
  dbusutils_get_object_data (iter, characteristic_properties, characteristic->object_path, BLUEZ_GATT_CHARACTERISTIC_INTERFACE, characteristic);
}

static void characteristic_get_uuid (void *user_data, DBusMessageIter *iter)
{
  characteristic_t *characteristic = (characteristic_t *) user_data;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, &characteristic->uuid);
}

static void characteristic_get_service (void *user_data, DBusMessageIter *iter)
{
  characteristic_t *characteristic = (characteristic_t *) user_data;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_OBJECT_PATH, &characteristic->service_path);
}

static void characteristic_get_flags (void *user_data, DBusMessageIter *iter)
{
  characteristic_t *characteristic = (characteristic_t *) user_data;
  DBusMessageIter array;

  dbus_message_iter_open_container (iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array);

  unsigned int flag_count = sizeof (characteristic_flags) / sizeof (characteristic_flags[0]);
  for (unsigned int i = 0; i < flag_count; i++)
  {
    if (utils_is_flag_set (characteristic->flags, characteristic_flags[i].enabled_bit))
    {
      dbusutils_iter_append_string (&array, DBUS_TYPE_STRING, characteristic_flags[i].flag_value);
    }
  }

  dbus_message_iter_close_container (iter, &array);
}

static void characteristic_get_value (void *user_data, DBusMessageIter *iter)
{
  characteristic_t *characteristic = (characteristic_t *) user_data;
  DBusMessageIter array;

  dbus_message_iter_open_container (iter, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &array);
  dbus_message_iter_append_fixed_array (&array, DBUS_TYPE_BYTE, &characteristic->value, characteristic->value_size);
  dbus_message_iter_close_container (iter, &array);
}

static void characteristic_set_value (characteristic_t *characteristic, const void *new_value, const uint32_t value_size)
{
  if (NULL == characteristic || new_value == NULL)
  {
    return;
  }

  free (characteristic->value);
  characteristic->value = NULL;
  characteristic->value_size = 0;

  characteristic->value = malloc (value_size);
  if (NULL == characteristic->value)
  {
    return;
  }
  memcpy (characteristic->value, new_value, value_size);
  characteristic->value_size = value_size;
}

// //Bluez methods
static DBusMessage *characteristic_read_value (void *user_data, DBusConnection *connection, DBusMessage *message)
{
  //TODO: parse message options  
  DBusMessage *reply = dbus_message_new_method_return (message);
  if (NULL == reply)
  {
    return NULL;
  }

  DBusMessageIter iter;
  dbus_message_iter_init_append (reply, &iter);
  characteristic_get_value (user_data, &iter);
  return reply;
}

static DBusMessage *characteristic_write_value (void *user_data, DBusConnection *connection, DBusMessage *message)
{
  DBusMessageIter args, array;
  dbus_message_iter_init (message, &args);

  int element_count = 0;
  uint8_t *new_value = NULL;

  dbus_message_iter_recurse (&args, &array);
  dbus_message_iter_get_fixed_array (&array, &new_value, &element_count);

  if (new_value == NULL || element_count < 0)
  {
    return NULL;
  }
  //TODO: parse message options 

  //set the value
  characteristic_set_value ((characteristic_t *) user_data, new_value, (uint32_t) element_count);

  DBusMessage *reply = dbus_message_new_method_return (message); //might need to return some sort of success, or maybe a lack of error is a success? ;) 
  return reply;
}

static DBusMessage *characteristic_start_notify (void *user_data, DBusConnection *connection, DBusMessage *message)
{
  characteristic_t *characteristic = (characteristic_t *) user_data;
  characteristic_set_notifying (characteristic, true);

  DBusMessage *reply = dbus_message_new_method_return (message);
  return reply;
}

static DBusMessage *characteristic_stop_notify (void *user_data, DBusConnection *connection, DBusMessage *message)
{
  characteristic_t *characteristic = (characteristic_t *) user_data;
  characteristic_set_notifying (characteristic, false);

  DBusMessage *reply = dbus_message_new_method_return (message);
  return reply;
}
