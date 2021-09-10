/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "descriptor.h"
#include "defines.h"
#include "dbusutils.h"
#include "utils.h"

static void descriptor_get_uuid (void *user_data, DBusMessageIter *iter);

static void descriptor_get_characteristic (void *user_data, DBusMessageIter *iter);

static void descriptor_get_flags (void *user_data, DBusMessageIter *iter);

static void descriptor_get_value (void *user_data, DBusMessageIter *iter);

static void descriptor_read_value (descriptor_t *descriptor, DBusMessageIter *iter);


static dbus_property_t descriptor_properties[] =
  {
    {BLE_PROPERTY_UUID, DBUS_TYPE_STRING_AS_STRING, descriptor_get_uuid},
    {BLE_PROPERTY_CHARACTERISTIC, DBUS_TYPE_OBJECT_PATH_AS_STRING, descriptor_get_characteristic},
    {BLE_PROPERTY_FLAGS, DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING, descriptor_get_flags},
    {BLE_PROPERTY_VALUE, DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_BYTE_AS_STRING, descriptor_get_value},
    DBUS_PROPERTY_NULL
  };

static dbus_method_t descriptor_methods[] =
  {
    DBUS_METHOD_NULL
  };

static object_flag_t descriptor_flags[] =
  {
    {DESCRIPTOR_FLAG_READ,                        DESCRIPTOR_FLAG_READ_ENABLED_BIT},
    {DESCRIPTOR_FLAG_WRITE,                       DESCRIPTOR_FLAG_WRITE_ENABLED_BIT},
    {DESCRIPTOR_FLAG_ENCRYPT_READ,                DESCRIPTOR_FLAG_ENCRYPT_READ_ENABLED_BIT},
    {DESCRIPTOR_FLAG_ENCRYPT_WRITE,               DESCRIPTOR_FLAG_ENCRYPT_WRITE_BIT},
    {DESCRIPTOR_FLAG_ENCRYPT_AUTHENTICATED_READ,  DESCRIPTOR_FLAG_ENCRYPT_AUTHENTICATED_READ_ENABLED_BIT},
    {DESCRIPTOR_FLAG_ENCRYTP_AUTHENTICATED_WRITE, DESCRIPTOR_FLAG_ENCRYPT_AUTHENTICATED_WRITE_ENABLED_BIT},
    {DESCRIPTOR_FLAG_SECURE_READ,                 DESCRIPTOR_FLAG_SECURE_READ_ENABLED_BIT},
    {DESCRIPTOR_FLAG_SECURE_WRITE,                DESCRIPTOR_FLAG_SECURE_WRITE_ENABLED_BIT},
    {DESCRIPTOR_FLAG_AUTHORIZE,                   DESCRIPTOR_FLAG_AUTHORIZE_ENABLED_BIT}
  };

void descriptor_init (descriptor_t *descriptor, const char *uuid, int origin)
{
  descriptor->origin = origin;
  descriptor->uuid = strdup (uuid);
  descriptor->characteristic_path = NULL;
  descriptor->object_path = NULL;

  descriptor->value = NULL;
  descriptor->value_size = 0;

  descriptor->flags = DESCRIPTOR_FLAGS_ALL_ENABLED;
  descriptor->next = NULL;
}

void descriptor_fini (descriptor_t *descriptor)
{
  if (NULL == descriptor)
  {
    return;
  }
  
  free (descriptor->object_path);
  free (descriptor->uuid);
  free (descriptor->characteristic_path);
  free (descriptor->value);
}


void descriptor_free (descriptor_t *descriptor)
{
  descriptor_fini (descriptor);
  free (descriptor);
}

bool descriptor_register (descriptor_t *descriptor)
{
  return dbusutils_register_object (global_dbus_connection, descriptor->object_path, descriptor_properties, descriptor_methods, descriptor);
}

//DBus methods
void descriptor_get_object (descriptor_t *descriptor, DBusMessageIter *iter)
{
  dbusutils_get_object_data (iter, descriptor_properties, descriptor->object_path, BLUEZ_GATT_DESCRIPTOR_INTERFACE, descriptor);
}

static void descriptor_get_uuid (void *user_data, DBusMessageIter *iter)
{
  descriptor_t *descriptor = (descriptor_t *) user_data;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, &descriptor->uuid);
}

static void descriptor_get_characteristic (void *user_data, DBusMessageIter *iter)
{
  descriptor_t *descriptor = (descriptor_t *) user_data;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_OBJECT_PATH, &descriptor->characteristic_path);
}

static void descriptor_get_flags (void *user_data, DBusMessageIter *iter)
{
  descriptor_t *descriptor = (descriptor_t *) user_data;
  DBusMessageIter array;

  dbus_message_iter_open_container (iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array);
  unsigned int flag_count = sizeof (descriptor_flags) / sizeof (descriptor_flags[0]);
  for (unsigned int i = 0; i < flag_count; i++)
  {
    if (utils_is_flag_set (descriptor->flags, descriptor_flags[i].enabled_bit))
    {
      dbusutils_iter_append_string (&array, DBUS_TYPE_STRING, descriptor_flags[i].flag_value);
    }
  }

  dbus_message_iter_close_container (iter, &array);
}

static void descriptor_get_value (void *user_data, DBusMessageIter *iter)
{
  descriptor_t *descriptor = (descriptor_t *) user_data;
  descriptor_read_value (descriptor, iter);
}

static void descriptor_read_value (descriptor_t *descriptor, DBusMessageIter *iter)
{
  DBusMessageIter array;

  dbus_message_iter_open_container (iter, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &array);
  dbus_message_iter_append_fixed_array (&array, DBUS_TYPE_BYTE, &descriptor->value, descriptor->value_size);
  dbus_message_iter_close_container (iter, &array);
}

// TODO: implement these functions
// void descriptor_get_all (descriptor_t *descriptor)
// {

// }

// //Bluez methods
// void descriptor_read_value (descriptor_t *descriptor)
// {

// }

// void descriptor_write_value (descriptor_t *descriptor)
// {

// }
