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
#include "utils.h"
#include "logger.h"

static void add_device_to_device_list (device_t *device);

static DBusMessage *device_get_managed_objects (void *device_ptr, DBusConnection *connection, DBusMessage *message);

static service_t *device_get_service (device_t *device, const char *service_uuid);

static bool device_init_controller (device_t *device);

static device_t *device_list_head = NULL;

static unsigned int device_count = 0;
static unsigned int controller_count = 1;

static dbus_method_t device_methods[] =
  {
    {DBUS_INTERFACE_OBJECT_MANAGER, DBUS_METHOD_GET_MANAGED_OBJECTS, device_get_managed_objects},
    DBUS_METHOD_NULL
  };

static void add_device_to_device_list (device_t *device)
{
  if (NULL == device)
  {
    return;
  }

  if (device_list_head == NULL)
  {
    device_list_head = device;
    return;
  }

  device->next = device_list_head;
  device_list_head = device;
}

//device constructors destructors
device_t *device_new (void)
{
  device_t *device = calloc (1, sizeof (*device));
  if (NULL == device)
  {
    return NULL;
  }

  return device;
}

void device_init (device_t *device, const char *device_name, int origin)
{
  device->origin = origin;
  device->device_name = strdup (device_name);
  device->controller = NULL;
  device->application_registered = false;
  device->initialised = false;
  device->services = NULL;
  device->service_count = 0;
  device->object_path = dbusutils_create_object_path (EMPTY_STRING, DEVICE_OBJECT_NAME, device_count);
  device->next = NULL;

  device->virtual_controller = NULL;

  device_count++;
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

  advertisement_terminate (&device->advertisement);

  vhci_close (device->virtual_controller);

  if (device->origin == ORIGIN_C)
  {
    service_t *tmp = NULL;
    while (device->services)
    {
      tmp = device->services->next;
      service_free (device->services);
      device->services = tmp;
    }

    free (device);
  }
}

static DBusMessage *device_get_managed_objects (void *device_ptr, DBusConnection *connection, DBusMessage *message)
{
  device_t *device = (device_t *) device_ptr;
  if (NULL == device || NULL == connection || NULL == message)
  {
    log_debug ("[%s:%u] Parameter was null", __FUNCTION__, __LINE__);
    return NULL;
  }

  DBusMessage *reply = dbus_message_new_method_return (message);
  if (reply == NULL)
  {
    log_debug ("[%s:%u] Could not create a dbus method return message", __FUNCTION__, __LINE__);
    return NULL;
  }

  //create the response - signature a{oa{sa{sv}}}
  DBusMessageIter iter, array;
  dbus_message_iter_init_append (reply, &iter);
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
  characteristic_t *characteristic = NULL;
  descriptor_t *descriptor = NULL;

  while (service)
  {
    service_get_object (service, &array);
    characteristic = service->characteristics;
    while (characteristic)
    {
      characteristic_get_object (characteristic, &array);
      descriptor = characteristic->descriptors;
      while (descriptor)
      {
        descriptor_get_object (descriptor, &array);
        descriptor = descriptor->next;
      }
      characteristic = characteristic->next;
    }
    service = service->next;
  }

  dbus_message_iter_close_container (&iter, &array);

  return reply;
}

static void on_register_application_reply (DBusPendingCall *pending_call, void *user_data)
{
  device_t *device = (device_t *) user_data;
  DBusMessage *reply = dbus_pending_call_steal_reply (pending_call);
  if (NULL == reply)
  {
    return;
  }

  if (dbus_message_get_type (reply) == DBUS_MESSAGE_TYPE_ERROR)
  {
    log_error ("Unable to Register device with bluez: (%s : %s)\n", dbus_message_get_error_name (reply), dbusutils_get_error_message_from_reply (reply));
    //TODO : remove device from list and unregister object
  }
  else
  {
    log_debug ("Successfully Registered device %s with bluez", device->device_name);
    device->application_registered = true;
  }

  dbus_message_unref (reply);
}

static bool device_register_with_bluez (device_t *device, DBusConnection *connection)
{
  if (NULL == device->controller)
  {
    log_debug ("[%s:%u] Device->contoller was NULL", __FUNCTION__, __LINE__);
    return false;
  }
  DBusMessage *message = dbus_message_new_method_call (BLUEZ_BUS_NAME, device->controller, BLUEZ_GATT_MANAGER_INTERFACE, BLUEZ_METHOD_REGISTER_APPLICATION);
  if (NULL == message)
  {
    log_error ("Register Application: Could not set up message");
    return false;
  }

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
  dbus_message_iter_close_container (&args, &dict);

  //send message
  DBusError error;
  dbus_error_init (&error);

  DBusPendingCall *pending_call = NULL;

  dbus_connection_send_with_reply (connection, message, &pending_call, DBUS_TIMEOUT_USE_DEFAULT);
  if (pending_call)
  {
    dbus_pending_call_set_notify (pending_call, on_register_application_reply, device, NULL);
  }

  if (message)
  {
    dbus_message_unref (message);
  }
  if (pending_call)
  {
    dbus_pending_call_unref (pending_call);
  }

  return true;
}

static bool device_init_controller (device_t *device)
{
  size_t required = snprintf (NULL, 0, BASE_ADAPTER_PATH"%u", controller_count) + 1;
  device->controller = malloc (required);
  sprintf (device->controller , BASE_ADAPTER_PATH"%u", controller_count);

  //create the virtual controller for the device
  device->virtual_controller = vhci_open(VHCI_TYPE_LE);
  if (NULL == device->virtual_controller)
  {
    return false;
  }
  log_info ("Created virtual controller hci%u for device %s", controller_count, device->device_name);

  msleep (HCI_WAKEUP_TIME); //give the hci some time to get up and running and for bluez to see that it is up
  controller_count++;
  return true;
}

//device manipulators - functions to create device, add services, characterisitcs etc
bool device_register (device_t *device)
{
  bool success = false;
  if (device_get_device (device->device_name))
  {
    log_warn ("Device with that name already exists");
    return false;
  }

  success = device_init_controller (device);
  if (!success)
  {
    log_error ("Failed to create device (%s) virtual controller", device->device_name);
    return false;
  }

  success = dbusutils_register_object (global_dbus_connection, device->object_path, NULL, device_methods, device);
  if (!success)
  {
    log_error ("Failed to register device (%s) with dbus", device->device_name);
    return false;
  }

  success = device_register_with_bluez (device, global_dbus_connection);
  if (!success)
  {
    log_error ("Failed to register device (%s) with bluez", device->device_name);
    return false;
  }

  //TODO: let the user set the manufacturer data and the key
  const uint8_t manufacturer_data[] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5};
  const unsigned int size = 24;
  const uint16_t key = 0xBEEF;

  //setup advertisement
  advertisement_init (
    &device->advertisement,
    dbusutils_create_object_path (device->object_path, ADVERTISEMENT_OBJECT_NAME, 0),
    &device->services,
    &device->device_name,
    key,
    manufacturer_data,
    size
  );

  success = advertisement_register (&device->advertisement);
  if (!success)
  {
    return false;
  }

  success = advertisement_register_with_bluez (&device->advertisement, device->controller, global_dbus_connection);
  if (!success)
  {
    return false;
  }

  add_device_to_device_list (device);
  device->initialised = true;

  log_info ("Simulating device %s", device->device_name);

  return true;
}

bool device_set_discoverable (device_t *device, bool discoverable)
{
  if (!device->initialised)
  {
    log_warn ("Device must be initialised to set its discoverability.");
    return false;
  }

  dbus_bool_t value;
  value = discoverable ? TRUE : FALSE;

  DBusMessage *reply = dbusutils_set_property_basic (
    global_dbus_connection,
    BLUEZ_BUS_NAME,
    device->controller,
    BLUEZ_ADAPTER_INTERFACE,
    BLUEZ_ADAPTER_PROPERTY_DISCOVERABLE,
    DBUS_TYPE_BOOLEAN,
    &value
  );

  if (NULL == reply)
  {
    return false;
  }

  log_info ("Device %s %s discoverable ", device->device_name, discoverable ? "" : "not");
  dbus_message_unref (reply);
  return true;
}

bool device_set_powered (device_t *device, bool powered)
{
  if (!device->initialised)
  {
    log_warn ("Device must be initialised to change its powerd state.");
    return false;
  }

  dbus_bool_t value;
  value = powered ? TRUE : FALSE;

  DBusMessage *reply = dbusutils_set_property_basic (
    global_dbus_connection,
    BLUEZ_BUS_NAME,
    device->controller,
    BLUEZ_ADAPTER_INTERFACE,
    BLUEZ_ADAPTER_PROPERTY_POWERED,
    DBUS_TYPE_BOOLEAN,
    &value
  );

  if (NULL == reply)
  {
    return false;
  }

  log_info ("Device %s powered %s", device->device_name, powered ? "on" : "off");
  dbus_message_unref (reply);
  return true;
}

device_t *device_get_device (const char *device_name)
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

static service_t *device_get_service (device_t *device, const char *service_uuid)
{
  service_t *service = device->services;
  while (service)
  {
    if (strcmp (service_uuid, service->uuid) == 0)
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
    log_debug ("[%s:%u] Device was NULL", __FUNCTION__, __LINE__);
    return false;
  }

  if (NULL != service->device_path)
  {
    log_warn ("Service already belongs to a device.");
    return false;
  }

  if (device_get_service (device, service->uuid))
  {
    log_warn ("Service %s already exists for device %s", service->uuid, device->device_name);
    return false;
  }

  service->object_path = dbusutils_create_object_path (device->object_path, SERVICE_OBJECT_NAME, device->service_count);
  if (!service_register (service))
  {
    free (service->object_path);
    return false;
  }
  service->device_path = strdup (device->object_path);

  service->next = device->services;
  device->services = service;

  device->service_count++;
  return true;
}
