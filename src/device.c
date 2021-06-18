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
#include "dbusutils.h"


static device_t *device_new (const char* device_name, const char* controller);
static void device_free (device_t *device);

static void add_device_to_device_list(device_t *device);
static void device_handle_unregister_device(DBusConnection *connection, void *data);
static DBusHandlerResult device_handle_dbus_message(DBusConnection *connection, DBusMessage *message, void *data);
static void device_get_managed_objects (device_t *device, DBusConnection *connection, DBusMessage *message );
static service_t* device_get_service (device_t *device, const char *service_uuid);

static device_t *device_list_head = NULL;
static unsigned int device_count = 0;

static DBusObjectPathVTable device_dbus_callbacks = {
  .unregister_function = device_handle_unregister_device,
  .message_function = device_handle_dbus_message,
};

//device list
void device_cleanup_devices (void)
{
  device_t *tmp = NULL;
  while (device_list_head)
  {
    tmp = device_list_head->next;
    device_free (device_list_head);
    device_list_head = tmp;
  }
  device_count = 0;
}

static void add_device_to_device_list(device_t *device)
{
  if (NULL == device)
  {
    return;
  }
  device_count++;

  if (device_list_head == NULL)
  {
    device_list_head = device;
    return;
  }

  device->next = device_list_head;
  device_list_head = device;
}

//device constructors destructors
static device_t *device_new (const char* device_name, const char* controller)
{
  device_t *device = calloc (1, sizeof (*device));
  if (NULL == device)
  {
    return NULL;
  }

  device->device_name = strdup (device_name);
  device->controller = strdup (controller);

  size_t required = snprintf(NULL, 0, "/dev%u",device_count) + 1;
  device->object_path = malloc(required);
  sprintf(device->object_path, "/dev%u",device_count);

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
  free (device->object_path);

  service_t *tmp = NULL;
  while (device->services)
  {
    tmp = device->services->next;
    service_free (device->services);
    device->services = tmp;
  }

  free (device);
}

//device object dbus functions
static void device_handle_unregister_device(DBusConnection *connection, void *data)
{

}

static DBusHandlerResult device_handle_dbus_message(DBusConnection *connection, DBusMessage *message, void *data)
{
  device_t *device = (device_t *) data;
  printf ("DEVICE MESSAGE: got dbus message sent to %s %s %s (device: %s) \n",
          dbus_message_get_destination(message),
          dbus_message_get_interface(message),
          dbus_message_get_path(message),
          device->device_name
          );

  if (dbus_message_is_method_call (message, DBUS_INTERFACE_OBJECT_MANAGER, DBUS_METHOD_GET_MANAGED_OBJECTS))
  {
    device_get_managed_objects (device, connection, message);
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

static void device_get_managed_objects (device_t *device, DBusConnection *connection, DBusMessage *message )
{
  printf("Device (%s) GetManagedObjects \n", device->device_name);
  // TODO: Implement get managed objects
}

static bool device_register_with_bluez(device_t *device, DBusConnection * connection)
{

  //init message
  DBusMessage *message = dbus_message_new_method_call(BLUEZ_BUS_NAME, device->controller, BLUEZ_GATT_MANAGER_INTERFACE, BLUEZ_METHOD_REGISTER_APPLICATION);
  if (NULL == message)
  {
    printf("Register Application: Could not set up message\n");
    return false;
  }

  printf("%s\n", device->object_path);

  //setup message
  DBusMessageIter args, dict;
  dbus_message_iter_init_append (message, &args);
  dbus_message_iter_append_basic (&args, DBUS_TYPE_OBJECT_PATH, &device->object_path);
  dbus_message_iter_open_container (&args, 
                                    DBUS_TYPE_ARRAY, 
                                    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING // signature "{sv}"
                                    DBUS_TYPE_STRING_AS_STRING
                                    DBUS_TYPE_VARIANT_AS_STRING
                                    DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
                                    &dict);
	// TODO: Could add options to dictionary 
	dbus_message_iter_close_container(&args, &dict);

  //send message
  DBusError error;
  dbus_error_init (&error);

  //TODO set up dbus_conneciton_send_with_reply so that we can get the response
  dbus_connection_send(connection, message, NULL);

  return true;
}

//device manipulators - functions to create device, add services, characterisitcs etc
bool device_add (const char* device_name)
{
  bool success = false;
  if (device_get_device (device_name))
  {
    printf ("Device with that name already exists\n");
    return false;
  }

  device_t *new_device = device_new (device_name, DEFAULT_CONTROLLER);

  success = dbusutils_register_object (global_dbus_connection,  new_device->object_path, &device_dbus_callbacks, new_device);
  if (!success)
  {
    device_free (new_device);
    return false;
  }

  success = device_register_with_bluez (new_device, global_dbus_connection);
  if (!success)
  {
    dbus_connection_unregister_object_path(global_dbus_connection, new_device->object_path);
    device_free (new_device);
    return false;
  }

  //init controller for device
  //TODO

  add_device_to_device_list(new_device);

  return true;
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

static service_t* device_get_service (device_t *device, const char *service_uuid)
{
  service_t *service = device->services;
  while (service)
  {
    if(strcmp (service_uuid, service->uuid) == 0)
    {
      return service;
    }
    service = service->next;
  }

  return NULL;
}

bool device_add_service (const char* device_name, service_t *service)
{
  device_t *device = device_get_device (device_name);
  if (NULL == device)
  {
    printf ("Could not add service to device %s, as the device doesn't exist\n", device_name);
    return false;
  }

  if (device_get_service (device, service->uuid))
  {
    return false;
  }

  service->next = device->services;
  device->services = service;
  return true;;
}

bool device_add_characteristic (const char* device_name,
                                const char *service_uuid,
                                characteristic_t *characteristic)
{
  device_t *device = device_get_device (device_name);
  if (NULL == device)
  {
    printf ("Could not add characteristic to device %s as the device doesn't exist\n", device_name);
    return false;
  }

  service_t *service = device_get_service (device, service_uuid);
  if (NULL == service)
  {
    printf ("Could not add characteristic to device %s as the service with uuid %s doesn't exist\n", device_name, service_uuid);
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
    printf ("Could not add descriptor to device %s as the device doesn't exist\n", device_name);
    return false;
  }

  service_t *service = device_get_service (device, service_uuid);
  if (NULL == service)
  {
    printf ("Could not add descriptor to device %s as the service with uuid %s doesn't exist\n", device_name, service_uuid);
    return false;
  }

  characteristic_t *characteristic = service_get_characteristic (service, characteristic_uuid);
  if (NULL == characteristic)
  {
    printf ("Could not add characteristic to device %s as the characteristic with uuid %s doesn't exist\n", device_name, characteristic_uuid);
    return false;
  }

  return characteristic_add_descriptor (characteristic, descriptor);
}
