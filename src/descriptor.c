/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <stdlib.h>
#include <string.h>

#include "descriptor.h"

static void descriptor_handle_unregister_device (DBusConnection *connection, void *data);
static DBusHandlerResult descriptor_handle_dbus_message (DBusConnection *connection, DBusMessage *message, void *data);

DBusObjectPathVTable descriptor_dbus_callbacks = {
  .unregister_function = descriptor_handle_unregister_device,
  .message_function = descriptor_handle_dbus_message,
};

descriptor_t *descriptor_new (const char *uuid, const char *characteristic_path)
{
  descriptor_t *new_descriptor = calloc (1, sizeof (*new_descriptor));
  if (NULL == new_descriptor)
  {
    return NULL;
  }

  new_descriptor->uuid = strdup (uuid);
  new_descriptor->characteristic_path = strdup (characteristic_path);
  new_descriptor->object_path = NULL;
  new_descriptor->value = NULL;
  new_descriptor->value_size = 0;
  new_descriptor->flags = 0xFFFF;
  new_descriptor->next = NULL;

  return new_descriptor;
}

void descriptor_free (descriptor_t *descriptor)
{
  if (NULL == descriptor)
  {
    return;
  }

  free (descriptor->uuid);
  free (descriptor->characteristic_path);
  free (descriptor->value);
  free (descriptor);
}

static void descriptor_handle_unregister_device (DBusConnection *connection, void *data)
{

}

static DBusHandlerResult descriptor_handle_dbus_message (DBusConnection *connection, DBusMessage *message, void *data)
{
  descriptor_t *descriptor = (descriptor_t*) data;
  printf ("DESCRIPTOR MESSAGE: got dbus message sent to %s %s %s (service: %s) \n",
          dbus_message_get_destination(message),
          dbus_message_get_interface(message),
          dbus_message_get_path(message),
          descriptor->uuid
          );

  return DBUS_HANDLER_RESULT_HANDLED;
}

//DBus methods
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
