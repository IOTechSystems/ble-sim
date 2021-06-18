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

static void add_device_to_device_list(device_t *device);
static void device_handle_unregister_device(DBusConnection *connection, void *data);
static DBusHandlerResult device_handle_dbus_message(DBusConnection *connection, DBusMessage *message, void *data);
static bool device_get_managed_objects (device_t *device, DBusConnection *connection, DBusMessage *message );
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
device_t *device_new (const char* device_name, const char* controller, service_t *services)
{
  device_t *device = calloc (1, sizeof (*device));
  if (NULL == device)
  {
    return NULL;
  }

  device->device_name = strdup (device_name);
  device->controller = strdup (controller);
  device->application_registered = false;
  device->services = services;
  device->service_count = 0;
  device->object_path = dbusutils_create_object_path (EMPTY_STRING, DEVICE_OBJECT_NAME, device_count);
  device->next = NULL;

  return device;
}

void device_free (device_t *device)
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

static bool device_get_managed_objects (device_t *device, DBusConnection *connection, DBusMessage *message )
{
  printf("Device (%s) GetManagedObjects \n", device->device_name);
  // TODO: Implement get managed objects
  if (NULL == device) 
  { 
    printf("%s: Device was null", __FUNCTION__);
    return false;
  }
  if (NULL == connection) 
  { 
    printf("%s: Connection was null", __FUNCTION__);
    return false;
  }
  if (NULL == message) 
  { 
    printf("%s: Message was null", __FUNCTION__);
    return false;
  }

  DBusMessage *reply = dbus_message_new_method_return(message);
	if (reply == NULL)
  {
    printf("%s: Could not create a method return message", __FUNCTION__);
		return false;
  }

  //create the response - signature a{oa{sa{sv}}}
  DBusMessageIter iter, array;
  dbus_message_iter_init_append (message, &iter);
	dbus_message_iter_open_container (
    &iter, 
    DBUS_TYPE_ARRAY, 
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING //{oa{sa{sv}}}
    DBUS_TYPE_OBJECT_PATH_AS_STRING
    DBUS_TYPE_ARRAY_AS_STRING
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_STRING_AS_STRING
    DBUS_TYPE_ARRAY_AS_STRING
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_STRING_AS_STRING
    DBUS_TYPE_VARIANT_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
    &array
  );

  service_t *service = device->services;
  // characteristic_t *characteristic = NULL;
  // descriptor_t *descriptor = NULL;

  while (service)
  {
    //call services func
    service_get_object(service, &array);


    // characteristic = service->characteristics;
    // while(characteristic)
    // {
    //   descriptor = characteristic->descriptors;
    //   while (descriptor)
    //   {

    //     descriptor = descriptor->next;
    //   }
    //   characteristic = characteristic->next;
    // }
    service = service->next;
  }

  dbus_message_iter_close_container (&iter, &array);

  //send reply
  dbus_connection_send(connection, message, NULL);

  return true;
}

static void on_register_application_reply(DBusPendingCall *pending_call, void *user_data)
{

  device_t *device = (device_t*) user_data;
	DBusMessage *reply;
	reply = dbus_pending_call_steal_reply(pending_call);
  if (NULL == reply)
  {
    return;
  }

	if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR)
  {
    printf("Unable to Register device with bluez: (%s : %s)\n", dbus_message_get_error_name(reply), dbusutils_get_error_message_from_reply (reply));
    //TODO : remove device from list and unregister
  }
  else
  {
    printf("Succesfully Registered device (%s) with bluez\n", device->device_name);
    device->application_registered = true;
  }

	dbus_message_unref(reply);
	dbus_pending_call_unref(pending_call);
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

  //setup message "oa{sv}""
  DBusMessageIter args, dict;
  dbus_message_iter_init_append (message, &args);
  dbus_message_iter_append_basic (&args, DBUS_TYPE_OBJECT_PATH, &device->object_path);
  dbus_message_iter_open_container (
    &args, 
    DBUS_TYPE_ARRAY, 
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING // signature "{sv}"
    DBUS_TYPE_STRING_AS_STRING
    DBUS_TYPE_VARIANT_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
    &dict
  );
	// TODO: Could add options to dictionary 
	dbus_message_iter_close_container(&args, &dict);

  //send message
  DBusError error;
  dbus_error_init (&error);

  DBusPendingCall *pending_call = NULL;

  dbus_connection_send_with_reply(connection, message, &pending_call, DBUS_TIMEOUT_USE_DEFAULT);
  if (pending_call)
  {
    dbus_pending_call_set_notify(pending_call, on_register_application_reply, NULL, NULL);
  }
  
  if (message) dbus_message_unref (message);
  if (pending_call) dbus_pending_call_unref (pending_call);

  return true;
}

//device manipulators - functions to create device, add services, characterisitcs etc
bool device_add (device_t* device)
{
  bool success = false;
  if (device_get_device (device->device_name))
  {
    printf ("Device with that name already exists\n");
    return false;
  }

  success = dbusutils_register_object (global_dbus_connection, device->object_path, &device_dbus_callbacks, device);
  if (!success)
  {
    return false;
  }

  success = device_register_with_bluez (device, global_dbus_connection);
  if (!success)
  {
    return false;
  }

  //init controller for device
  //TODO
  add_device_to_device_list(device);

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

bool device_add_service (device_t *device, service_t *service)
{
  if (NULL == device)
  {
    printf ("Device was null\n");
    return false;
  }

  if (device_get_service (device, service->uuid)) 
  {
    printf ("Service %s already exists for device %s", service->uuid, device->device_name);
    return false;
  }

  char *service_object_path = dbusutils_create_object_path (device->object_path, SERVICE_OBJECT_NAME, device->service_count);
  if (!dbusutils_register_object (global_dbus_connection, service_object_path, &service_dbus_callbacks, service))
  {
    free (service_object_path);
    return false;
  }
  service->object_path = service_object_path;

  service->next = device->services;
  device->services = service;

  device->service_count++;
  return true;
}

bool device_add_characteristic (device_t *device,
                                const char *service_uuid,
                                characteristic_t *characteristic)
{
  if (NULL == device)
  {
    printf ("Device was null\n");
    return false;
  }

  service_t *service = device_get_service (device, service_uuid);
  if (NULL == service)
  {
    printf ("Could not add characteristic to device %s as the service with uuid %s doesn't exist\n", device->device_name, service_uuid);
    return false;
  }

  return service_add_characteristic (service, characteristic);
}

bool device_add_descriptor (device_t *device,
                            const char *service_uuid,
                            const char *characteristic_uuid,
                            descriptor_t *descriptor)
{
  if (NULL == device)
  {
    printf ("Device was null\n");
    return false;
  }

  service_t *service = device_get_service (device, service_uuid);
  if (NULL == service)
  {
    printf ("Could not add descriptor to device %s as the service with uuid %s doesn't exist\n", device->device_name, service_uuid);
    return false;
  }

  characteristic_t *characteristic = service_get_characteristic (service, characteristic_uuid);
  if (NULL == characteristic)
  {
    printf ("Could not add characteristic to device %s as the characteristic with uuid %s doesn't exist\n", device->device_name, characteristic_uuid);
    return false;
  }

  return characteristic_add_descriptor (characteristic, descriptor);
}
