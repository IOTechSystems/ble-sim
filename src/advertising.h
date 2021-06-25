/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_ADVERTISING_H
#define BLE_SIM_ADVERTISING_H

#include <stdint.h>
#include <dbus/dbus.h>

#include "service.h"
#include "dbusutils.h"

extern DBusConnection *global_dbus_connection;

typedef struct advertisement_data_t
{
  uint8_t data[ADVERTISEMENT_DATA_MAX_SIZE];
  uint16_t length;
} advertisement_data_t;

typedef struct manufacturer_data_t
{
  uint16_t id;
  advertisement_data_t data;
} manufacturer_data_t;

typedef struct service_data_t
{
  char *uuid;
  advertisement_data_t data;
} service_data_t;

typedef struct data_t
{
  uint8_t type;
  advertisement_data_t data;
} data_t;

typedef struct advertisement_t
{
  bool registered;
  char *object_path;
  char *type;
  service_t **services; //pointer to the devices services pointer
  manufacturer_data_t manufacturer_data;
  //TODO: add solicit uuids
  service_data_t service_data;
  data_t data;
  bool discoverable;
  uint16_t discoverable_timeout;
  //includes; //TODO: turn this in to flags eg in a uint8_t for now this is just hardcoded in get function
  char **local_name; //pointer to device name
  uint16_t appearance;
  uint16_t duration; //duration of the advertisement in seconds
  uint16_t timeout;
  char *secondary_channel; //TODO: turn this into a enum
  uint32_t min_interval;
  uint32_t max_interval;
  int16_t tx_power; //TODO: add this in
} advertisement_t;

/**
 * Initialised advertisement values 
 * @param advertisement pointer to the advertisement
 * @param object_path object path of the advertisement
 * @param services pointer to a pointer for list of services
 * @param device_name pointer to the device name pointer
 * @param manufacturer_key the manufacturer key
 * @param manufacturer_data manufacturer advertisement data
 * @param manufacturer_data_size size of manufacturer_data
 **/
void advertisement_init (
  advertisement_t *advertisement,
  char *object_path,
  service_t **services,
  char **device_name,
  uint16_t manufacturer_key,
  const uint8_t *manufacturer_data,
  unsigned int manufacturer_data_size
);

/**
 * Terminates the advertisement and frees all of its allocated mems
 * @param advertisement adveritsement to terminate
 */
void advertisement_terminate (advertisement_t *advertisement);

/**
 * Registers the advertisement object with dbus
 * @param advertisement pointer to the advertisement
 * @return success true/false
 **/
bool advertisement_register (advertisement_t *advertisement);

/**
 * Registers the advertisement object with bluez advertisement manager
 * @param advertisement pointer to the advertisement
 * @param controller_path path of the bluez controller to register the advertisement with
 * @return success true/false
 **/
bool advertisement_register_with_bluez (
  advertisement_t *advertisement,
  const char *controller_path,
  DBusConnection *connection
);

#endif //BLE_SIM_ADVERTISING_H