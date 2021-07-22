/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "advertising.h"
#include "dbusutils.h"
#include "logger.h"

static DBusMessage *advertisement_release (void *advertisement_ptr, DBusConnection *connection, DBusMessage *message);

//TODO: enable commented out properties and their respective functions/entries in the property table
static void advertisement_get_type (void *advertisement_ptr, DBusMessageIter *iter);

//static void advertisement_get_service_uuids(void *advertisement_ptr, DBusMessageIter *iter);
static void advertisement_get_manufacturer_data (void *advertisement_ptr, DBusMessageIter *iter);

//static void advertisement_get_service_data(void *advertisement_ptr, DBusMessageIter *iter);
//static void advertisement_get_data(void *advertisement_ptr, DBusMessageIter *iter);
static void advertisement_get_discoverable (void *advertisement_ptr, DBusMessageIter *iter);

static void advertisement_get_discoverable_timeout (void *advertisement_ptr, DBusMessageIter *iter);

static void advertisement_get_includes (void *advertisement_ptr, DBusMessageIter *iter);

//static void advertisement_get_local_name(void *advertisement_ptr, DBusMessageIter *iter);
//static void advertisement_get_appearance(void *advertisement_ptr, DBusMessageIter *iter);
static void advertisement_get_duration (void *advertisement_ptr, DBusMessageIter *iter);

static void advertisement_get_timeout (void *advertisement_ptr, DBusMessageIter *iter);

//static void advertisement_get_secondary_channel(void *advertisement_ptr, DBusMessageIter *iter);
static void advertisement_get_min_interval (void *advertisement_ptr, DBusMessageIter *iter);

static void advertisement_get_max_interval (void *advertisement_ptr, DBusMessageIter *iter);
//static void advertisement_get_tx_power(void *advertisement_ptr, DBusMessageIter *iter);

static dbus_property_t advertisement_properties[] =
  {
    {BLE_PROPERTY_TYPE, DBUS_TYPE_STRING_AS_STRING, advertisement_get_type},
    //{BLE_PROPERTY_SERVICE_UUIDS,  DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING, advertisement_get_service_uuids},
    {BLE_PROPERTY_MANUFACTURER_DATA,
     DBUS_TYPE_ARRAY_AS_STRING DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_UINT16_AS_STRING DBUS_TYPE_VARIANT_AS_STRING DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
     advertisement_get_manufacturer_data},
    //{BLE_PROPERTY_SOLICIT_UUIDS, , },
    //{BLE_PROPERTY_SERVICE_DATA, DBUS_TYPE_ARRAY_AS_STRING DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING DBUS_DICT_ENTRY_END_CHAR_AS_STRING, advertisement_get_service_data},
    //{BLE_PROPERTY_DATA, DBUS_TYPE_ARRAY_AS_STRING DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_BYTE_AS_STRING DBUS_TYPE_VARIANT_AS_STRING DBUS_DICT_ENTRY_END_CHAR_AS_STRING, advertisement_get_data},
    {BLE_PROPERTY_DISCOVERABLE, DBUS_TYPE_BOOLEAN_AS_STRING, advertisement_get_discoverable},
    {BLE_PROPERTY_DISCOVERABLE_TIMEOUT, DBUS_TYPE_UINT16_AS_STRING, advertisement_get_discoverable_timeout},
    {BLE_PROPERTY_INCLUDES, DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING, advertisement_get_includes},
    //{BLE_PROPERTY_LOCAL_NAME, DBUS_TYPE_STRING_AS_STRING, advertisement_get_local_name},
    //{BLE_PROPERTY_APPEARANCE, DBUS_TYPE_UINT16_AS_STRING, advertisement_get_appearance},
    {BLE_PROPERTY_DURATION, DBUS_TYPE_UINT16_AS_STRING, advertisement_get_duration},
    {BLE_PROPERTY_TIMEOUT, DBUS_TYPE_UINT16_AS_STRING, advertisement_get_timeout},
    //{BLE_PROPERTY_SECONDARY_CHANNEL, DBUS_TYPE_STRING_AS_STRING, advertisement_get_secondary_channel},
    {BLE_PROPERTY_MIN_INTERVAL, DBUS_TYPE_UINT32_AS_STRING, advertisement_get_min_interval},
    {BLE_PROPERTY_MAX_INTERVAL, DBUS_TYPE_UINT32_AS_STRING, advertisement_get_max_interval},
    //{BLE_PROPERTY_TX_POWER, DBUS_TYPE_INT16_AS_STRING, advertisement_get_tx_power},
    DBUS_PROPERTY_NULL
  };

static dbus_method_t advertisement_methods[] =
  {
    {BLUEZ_LE_ADVERTISEMENT_INTERFACE, BLUEZ_METHOD_RELEASE, advertisement_release},
    DBUS_METHOD_NULL
  };

void advertisement_init (
  advertisement_t *advertisement,
  char *object_path,
  service_t **services,
  char **device_name,
  uint16_t manufacturer_key,
  const uint8_t *manufacturer_data,
  unsigned int manufacturer_data_size
)
{
  advertisement->registered = false;
  advertisement->object_path = object_path;
  advertisement->services = services;
  advertisement->local_name = device_name;

  advertisement->type = strdup (ADVERTISEMENT_TYPE_DEFAULT);

  advertisement->manufacturer_data.id = manufacturer_key;
  advertisement->manufacturer_data.data.length = min(manufacturer_data_size, ADVERTISEMENT_DATA_MAX_SIZE);
  memcpy (&advertisement->manufacturer_data.data.data, manufacturer_data, min(manufacturer_data_size, ADVERTISEMENT_DATA_MAX_SIZE));

  //TODO: init service_data and data once we have implemented
/*  advertisement->data.data.length = 0;
  advertisement->service_data.uuid = NULL;
  advertisement->service_data.data.length = 0;*/

  advertisement->discoverable = ADVERTISEMENT_DISCOVERABLE_DEFAULT;
  advertisement->discoverable_timeout = ADVERTISEMENT_DISCOVERABLE_TIMEOUT_DEFAULT;
  advertisement->appearance = UINT16_MAX;
  advertisement->duration = ADVERTISEMENT_DURATION_DEFAULT;
  advertisement->timeout = ADVERTISEMENT_TIMEOUT_DEFAULT;
  advertisement->secondary_channel = strdup (ADVERTISEMENT_SECONDARY_CHANNEL_DEFAULT);
  advertisement->min_interval = ADVERTISEMENT_MIN_INTERVAL_DEFAULT;
  advertisement->max_interval = ADVERTISEMENT_MAX_INTERVAL_DEFAULT;
  advertisement->tx_power = ADVERTISEMENT_TX_POWER_DEFAULT;
}

void advertisement_terminate (advertisement_t *advertisement)
{
  if (NULL == advertisement)
  {
    return;
  }

  free (advertisement->object_path);
  free (advertisement->type);
  free (advertisement->secondary_channel);
}

bool advertisement_register (advertisement_t *advertisement)
{
  return dbusutils_register_object (
    global_dbus_connection,
    advertisement->object_path,
    advertisement_properties,
    advertisement_methods,
    advertisement
  );
}

static void on_register_advert_reply (DBusPendingCall *pending_call, void *user_data)
{
  advertisement_t *advertisement = (advertisement_t *) user_data;
  DBusMessage *reply = dbus_pending_call_steal_reply (pending_call);
  if (NULL == reply)
  {
    return;
  }

  if (dbus_message_get_type (reply) == DBUS_MESSAGE_TYPE_ERROR)
  {
    log_error (
      "Unable to Register advert for device (%s) with bluez: (%s : %s)", 
      *advertisement->local_name, 
      dbus_message_get_error_name (reply),
      dbusutils_get_error_message_from_reply (reply)
    );
    //TODO : do something more with the error
  }
  else
  {
    log_debug ("Successfully Registered %s's advertisement with bluez", *advertisement->local_name);
    advertisement->registered = true;
  }

  dbus_message_unref (reply);
}

bool advertisement_register_with_bluez (
  advertisement_t *advertisement,
  const char *controller_path,
  DBusConnection *connection
)
{
  DBusMessage *message = dbus_message_new_method_call (
    BLUEZ_BUS_NAME,
    controller_path,
    BLUEZ_LE_ADVERTISING_MANAGER_INTERFACE,
    BLUEZ_METHOD_REGISTER_ADVERTISEMENT
  );

  if (NULL == message)
  {
    log_error ("Register Advertisement: Could not set up message");
    return false;
  }

  //setup message "oa{sv}"
  DBusMessageIter args, dict;
  dbus_message_iter_init_append (message, &args);
  dbus_message_iter_append_basic (&args, DBUS_TYPE_OBJECT_PATH, &advertisement->object_path);
  dbus_message_iter_open_container (
    &args,
    DBUS_TYPE_ARRAY,
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING // signature "{sv}"
    DBUS_TYPE_STRING_AS_STRING
    DBUS_TYPE_VARIANT_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
    &dict
  );
  //TODO: add options here  
  dbus_message_iter_close_container (&args, &dict);

  DBusError error;
  dbus_error_init (&error);

  DBusPendingCall *pending_call = NULL;

  dbus_connection_send_with_reply (connection, message, &pending_call, DBUS_TIMEOUT_USE_DEFAULT);
  if (pending_call)
  {
    dbus_pending_call_set_notify (pending_call, on_register_advert_reply, advertisement, NULL);
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

static DBusMessage *advertisement_release (
  void *advertisement_ptr,
  DBusConnection *connection,
  DBusMessage *message
)
{
  return NULL;
}

static void advertisement_get_type (void *advertisement_ptr, DBusMessageIter *iter)
{
  advertisement_t *advertisement = (advertisement_t *) advertisement_ptr;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, &advertisement->type);
}

// static void advertisement_get_service_uuids(void *advertisement_ptr, DBusMessageIter *iter)
// {
//   advertisement_t *advertisement = (advertisement_t*) advertisement_ptr; 

//   DBusMessageIter array;
//   dbus_message_iter_open_container (iter, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING_AS_STRING, &array);

//   service_t *service = *advertisement->services;
//   while (service)
//   {
//     dbus_message_iter_append_basic (&array, DBUS_TYPE_STRING, &service->uuid);
//     service = service->next;
//   }
//   dbus_message_iter_close_container (iter, &array);
// }

static void advertisement_get_manufacturer_data (void *advertisement_ptr, DBusMessageIter *iter)
{
  advertisement_t *advertisement = (advertisement_t *) advertisement_ptr;

  DBusMessageIter dict;
  dbus_message_iter_open_container (
    iter,
    DBUS_TYPE_ARRAY,
    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
    DBUS_TYPE_UINT16_AS_STRING
    DBUS_TYPE_VARIANT_AS_STRING
    DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
    &dict
  );

  uint8_t *data_ptr = advertisement->manufacturer_data.data.data;

  dbusutils_iter_append_dict_entry_fixed_array (
    &dict,
    DBUS_TYPE_UINT16,
    &advertisement->manufacturer_data.id,
    DBUS_TYPE_BYTE,
    &data_ptr,
    (unsigned int) advertisement->manufacturer_data.data.length
  );

  dbus_message_iter_close_container (iter, &dict);
}

// static void advertisement_get_service_data(void *advertisement_ptr, DBusMessageIter *iter)
// {
//   advertisement_t *advertisement = (advertisement_t*) advertisement_ptr;

//   DBusMessageIter dict;
//   dbus_message_iter_open_container (
//     iter, 
//     DBUS_TYPE_ARRAY, 
//     DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING 
//     DBUS_TYPE_STRING_AS_STRING 
//     DBUS_TYPE_VARIANT_AS_STRING 
//     DBUS_DICT_ENTRY_END_CHAR_AS_STRING, 
//     &dict
//   );

//   uint8_t *data_ptr = advertisement->service_data.data.data;

//   dbusutils_iter_append_dict_entry_fixed_array(
//     &dict, 
//     DBUS_TYPE_STRING, 
//     &advertisement->service_data.uuid, 
//     DBUS_TYPE_BYTE, 
//     &data_ptr,
//     (unsigned int) advertisement->service_data.data.length
//   );

//   dbus_message_iter_close_container (iter, &dict);
// }

// static void advertisement_get_data(void *advertisement_ptr, DBusMessageIter *iter)
// {
//   advertisement_t *advertisement = (advertisement_t*) advertisement_ptr;
//   DBusMessageIter dict;
//   dbus_message_iter_open_container (
//     iter, 
//     DBUS_TYPE_ARRAY, 
//     DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING 
//     DBUS_TYPE_BYTE_AS_STRING 
//     DBUS_TYPE_VARIANT_AS_STRING 
//     DBUS_DICT_ENTRY_END_CHAR_AS_STRING, 
//     &dict
//   );

//   uint8_t *data_ptr = advertisement->service_data.data.data;

//   dbusutils_iter_append_dict_entry_fixed_array(
//     &dict, 
//     DBUS_TYPE_BYTE, 
//     &advertisement->data.type, 
//     DBUS_TYPE_BYTE, 
//     &data_ptr,
//     (unsigned int) advertisement->data.data.length
//   );

//   dbus_message_iter_close_container (iter, &dict);
// }

static void advertisement_get_discoverable (void *advertisement_ptr, DBusMessageIter *iter)
{
  advertisement_t *advertisement = (advertisement_t *) advertisement_ptr;
  dbus_bool_t value = advertisement->discoverable ? TRUE : FALSE;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_BOOLEAN, &value);
}

static void advertisement_get_discoverable_timeout (void *advertisement_ptr, DBusMessageIter *iter)
{
  advertisement_t *advertisement = (advertisement_t *) advertisement_ptr;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT16, &advertisement->discoverable_timeout);
}

static void advertisement_get_includes (void *advertisement_ptr, DBusMessageIter *iter)
{
  //advertisement_t *advertisement = (advertisement_t*) advertisement_ptr;  
  DBusMessageIter array;

  dbus_message_iter_open_container (iter, DBUS_TYPE_ARRAY, DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_STRING_AS_STRING, &array);


  //TODO: make this correctly show the includes based on what is set
  /*const char* tx_power = "tx-power";
  const char* appearance = "appearance";
  const char* local_name = "local-name";

  dbus_message_iter_append_basic (&array, DBUS_TYPE_STRING, &tx_power);
  dbus_message_iter_append_basic (&array, DBUS_TYPE_STRING, &appearance);
  dbus_message_iter_append_basic (&array, DBUS_TYPE_STRING, &local_name);*/

  dbus_message_iter_close_container (iter, &array);
}

// static void advertisement_get_local_name(void *advertisement_ptr, DBusMessageIter *iter)
// {
//   advertisement_t *advertisement = (advertisement_t*) advertisement_ptr;  
//   dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, advertisement->local_name);
// }

// static void advertisement_get_appearance(void *advertisement_ptr, DBusMessageIter *iter)
// {
//   advertisement_t *advertisement = (advertisement_t*) advertisement_ptr;  
//   dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT16, &advertisement->appearance);
//}

static void advertisement_get_duration (void *advertisement_ptr, DBusMessageIter *iter)
{
  advertisement_t *advertisement = (advertisement_t *) advertisement_ptr;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT16, &advertisement->duration);
}

static void advertisement_get_timeout (void *advertisement_ptr, DBusMessageIter *iter)
{
  advertisement_t *advertisement = (advertisement_t *) advertisement_ptr;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT16, &advertisement->timeout);
}

// static void advertisement_get_secondary_channel(void *advertisement_ptr, DBusMessageIter *iter)
// {
//   advertisement_t *advertisement = (advertisement_t*) advertisement_ptr;  
//   dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, &advertisement->secondary_channel);
// }

static void advertisement_get_min_interval (void *advertisement_ptr, DBusMessageIter *iter)
{
  advertisement_t *advertisement = (advertisement_t *) advertisement_ptr;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT32, &advertisement->min_interval);
}

static void advertisement_get_max_interval (void *advertisement_ptr, DBusMessageIter *iter)
{
  advertisement_t *advertisement = (advertisement_t *) advertisement_ptr;
  dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT32, &advertisement->max_interval);
}

// static void advertisement_get_tx_power(void *advertisement_ptr, DBusMessageIter *iter)
// {
//   advertisement_t *advertisement = (advertisement_t*) advertisement_ptr;  
//   dbus_message_iter_append_basic (iter, DBUS_TYPE_INT16, &advertisement->tx_power);
// }
