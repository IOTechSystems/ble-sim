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

static void characteristic_handle_unregister_device (DBusConnection *connection, void *data);
static DBusHandlerResult characteristic_handle_dbus_message (DBusConnection *connection, DBusMessage *message, void *data);

DBusObjectPathVTable characteristic_dbus_callbacks = {
  .unregister_function = characteristic_handle_unregister_device,
  .message_function = characteristic_handle_dbus_message,
};

characteristic_t *characteristic_new (const char *uuid, const char *service_path, descriptor_t *descriptors)
{
  characteristic_t *new_characteristic = calloc (1, sizeof (*new_characteristic));
  if (NULL == new_characteristic)
  {
    return NULL;
  }

  new_characteristic->uuid = strdup (uuid);
  new_characteristic->service_path = strdup (service_path);
  new_characteristic->object_path = NULL;
  new_characteristic->value = NULL;
  new_characteristic->value_size = 0;
  new_characteristic->flags = 0xFFFFFFFF; //all enabled for now
  new_characteristic->descriptors = descriptors;
  new_characteristic->descriptor_count = 0;
  new_characteristic->next = NULL;

  return new_characteristic;
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

  descriptor_t *tmp = NULL;
  while (characteristic->descriptors)
  {
    tmp = characteristic->descriptors->next;
    descriptor_free (characteristic->descriptors);
    characteristic->descriptors = tmp;
  }

  free (characteristic);
}

static void characteristic_handle_unregister_device (DBusConnection *connection, void *data)
{

}

static DBusHandlerResult characteristic_handle_dbus_message (DBusConnection *connection, DBusMessage *message, void *data)
{
  characteristic_t *characterisitc = (characteristic_t *) data;
  printf ("CHARACTERISTIC MESSAGE: got dbus message sent to %s %s %s (service: %s) \n",
          dbus_message_get_destination(message),
          dbus_message_get_interface(message),
          dbus_message_get_path(message),
          characterisitc->uuid
          );

  return DBUS_HANDLER_RESULT_HANDLED;
}

descriptor_t *characteristic_get_descriptor (characteristic_t *characteristic, const char *descriptor_uuid)
{
  descriptor_t *descriptor = characteristic->descriptors;
  while (descriptor)
  {
    if (strcmp(descriptor_uuid, descriptor->uuid) == 0)
    {
      return descriptor;
    }
    descriptor = descriptor->next;
  }

  return NULL; 
}

bool characteristic_add_descriptor (characteristic_t *characteristic, descriptor_t *descriptor)
{
  if (characteristic_get_descriptor (characteristic, descriptor->uuid))
  {
    return false;
  }

  char *descriptor_object_path = dbusutils_create_object_path (characteristic->object_path, DESCRIPTOR_OBJECT_NAME, characteristic->descriptor_count);
  if (!dbusutils_register_object (global_dbus_connection, descriptor_object_path, &descriptor_dbus_callbacks, characteristic))
  {
    free (descriptor_object_path);
    return false;
  }
  characteristic->object_path = descriptor_object_path;

  descriptor->next = characteristic->descriptors;
  characteristic->descriptors = descriptor;
  characteristic->descriptor_count++;
  return true;
}

// //DBus Methods
// void characteristic_get_all (characteristic_t *characteristic)
// {

// }

// void characteristic_properties_changed (characteristic_t *characteristic)
// {

// }

// //Bluez methods
// void characteristic_read_value (characteristic_t *characteristic)
// {

// }

// void characteristic_write_value (characteristic_t *characteristic)
// {

// }

// void characteristic_aquire_write (characteristic_t *characteristic)
// {

// }

// void characteristic_aquire_notify (characteristic_t *characteristic)
// {

// }

// void characteristic_start_notify (characteristic_t *characteristic)
// {

// }

// void characteristic_stop_notify (characteristic_t *characteristic)
// {

// }
