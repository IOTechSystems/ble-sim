/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_DEFINES_H
#define BLE_SIM_DEFINES_H

#include <stdbool.h>

#define SIM_ARGS_OPTION_SCRIPT "--script"
#define SIM_ARGS_OPTION_HELP "--help"

#define ORIGIN_C 1
#define ORIGIN_LUA 3

#define BLE_SIM_TICK_RATE_MS 100//ms
#define BLE_SIM_SERVICE_NAME "org.blesim"
#define DEFAULT_ADAPTER "/org/bluez/hci1"

#define UUID_LENGTH 32
#define DEVICE_PATH_LENGTH 22

#define MAX_DEVICE_COUNT 1 //TODO: add functionality to support more than one device

#define EMPTY_STRING ""
#define ROOT_PATH "/"
#define DBUS_PROPERTY_NULL { NULL, NULL, NULL }
#define DBUS_METHOD_NULL { NULL, NULL, NULL }

#define DEVICE_OBJECT_NAME "dev"
#define SERVICE_OBJECT_NAME "serv"
#define CHARACTERISTIC_OBJECT_NAME "char"
#define DESCRIPTOR_OBJECT_NAME "desc"
#define ADVERTISEMENT_OBJECT_NAME "advrt"

#define ADVERTISEMENT_TYPE_DEFAULT "peripheral"
#define ADVERTISEMENT_SECONDARY_CHANNEL_DEFAULT "1M"
#define ADVERTISEMENT_DURATION_DEFAULT 5
#define ADVERTISEMENT_TIMEOUT_DEFAULT 0
#define ADVERTISEMENT_DISCOVERABLE_TIMEOUT_DEFAULT 0
#define ADVERTISEMENT_DISCOVERABLE_DEFAULT true
#define ADVERTISEMENT_MIN_INTERVAL_DEFAULT 1000
#define ADVERTISEMENT_MAX_INTERVAL_DEFAULT 5000
#define ADVERTISEMENT_TX_POWER_DEFAULT -127

#define ADVERTISEMENT_DATA_MAX_SIZE 24

#define BLE_PROPERTY_UUID "UUID"
#define BLE_PROPERTY_PRIMARY "Primary"
#define BLE_PROPERTY_DEVICE "Device"
#define BLE_PROPERTY_SERVICE "Service"
#define BLE_PROPERTY_CHARACTERISTIC "Characteristic"
#define BLE_PROPERTY_VALUE "Value"
#define BLE_PROPERTY_FLAGS "Flags"
#define BLE_PROPERTY_TYPE "Type"
#define BLE_PROPERTY_SERVICE_UUIDS "ServiceUUIDs"
#define BLE_PROPERTY_MANUFACTURER_DATA "ManufacturerData"
#define BLE_PROPERTY_SOLICIT_UUIDS "SolicitUUIDs"
#define BLE_PROPERTY_SERVICE_DATA "ServiceData"
#define BLE_PROPERTY_DATA "Data"
#define BLE_PROPERTY_DISCOVERABLE "Discoverable"
#define BLE_PROPERTY_DISCOVERABLE_TIMEOUT "DiscoverableTimeout"
#define BLE_PROPERTY_INCLUDES "Includes"
#define BLE_PROPERTY_LOCAL_NAME "LocalName"
#define BLE_PROPERTY_APPEARANCE "Appearance"
#define BLE_PROPERTY_DURATION "Duration"
#define BLE_PROPERTY_TIMEOUT "Timeout"
#define BLE_PROPERTY_SECONDARY_CHANNEL "SecondaryChannel"
#define BLE_PROPERTY_MIN_INTERVAL "MinInterval"
#define BLE_PROPERTY_MAX_INTERVAL "MaxInterval"
#define BLE_PROPERTY_TX_POWER "TxPower"

#define DBUS_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"
#define DBUS_SIGNAL_PROPERTIES_CHANGED "PropertiesChanged"
#define DBUS_METHOD_SET "Set"
#define DBUS_METHOD_GET "Get"
#define DBUS_METHOD_GET_ALL "GetAll"

#define DBUS_INTERFACE_OBJECT_MANAGER "org.freedesktop.DBus.ObjectManager"
#define DBUS_METHOD_GET_MANAGED_OBJECTS "GetManagedObjects"

#define BLUEZ_BUS_NAME "org.bluez"
#define BLUEZ_ADAPTER_INTERFACE "org.bluez.Adapter1"
#define BLUEZ_GATT_MANAGER_INTERFACE "org.bluez.GattManager1"
#define BLUEZ_GATT_SERVICE_INTERFACE "org.bluez.GattService1"
#define BLUEZ_GATT_CHARACTERISTIC_INTERFACE "org.bluez.GattCharacteristic1"
#define BLUEZ_GATT_DESCRIPTOR_INTERFACE "org.bluez.GattDescriptor1"
#define BLUEZ_LE_ADVERTISING_MANAGER_INTERFACE "org.bluez.LEAdvertisingManager1"
#define BLUEZ_LE_ADVERTISEMENT_INTERFACE "org.bluez.LEAdvertisement1"

#define BLUEZ_ADAPTER_PROPERTY_POWERED "Powered"
#define BLUEZ_ADAPTER_PROPERTY_DISCOVERABLE "Discoverable"

#define BLUEZ_METHOD_RELEASE "Release"
#define BLUEZ_METHOD_REGISTER_APPLICATION "RegisterApplication"
#define BLUEZ_METHOD_REGISTER_ADVERTISEMENT "RegisterAdvertisement"
#define BLUEZ_METHOD_READ_VALUE "ReadValue"
#define BLUEZ_METHOD_WRITE_VALUE "WriteValue"
#define BLUEZ_METHOD_START_NOTIFY "StartNotify"
#define BLUEZ_METHOD_STOP_NOTIFY "StopNotify"

#define DEFAULT_TIMEOUT 1000

//flags
#define CHARACTERISTIC_FLAGS_ALL_ENABLED 0x01FFFFFF
#define CHARACTERISTIC_FLAG_BROADCAST "broadcast"
#define CHARACTERISTIC_FLAG_BROADCAST_ENABLED_BIT (1 << 0)
#define CHARACTERISTIC_FLAG_READ "read"
#define CHARACTERISTIC_FLAG_READ_ENABLED_BIT (1 << 1)
#define CHARACTERISTIC_FLAG_WRITE_WITHOUT_RESPONSE "write-without-response"
#define CHARACTERISTIC_FLAG_WRITE_WITHOUT_RESPONSE_ENABLED_BIT (1 << 2)
#define CHARACTERISTIC_FLAG_WRITE "write"
#define CHARACTERISTIC_FLAG_WRITE_ENABLED_BIT (1 << 3)
#define CHARACTERISTIC_FLAG_NOTIFY "notify"
#define CHARACTERISTIC_FLAG_NOTIFY_ENABLED_BIT (1 << 4)
#define CHARACTERISTIC_FLAG_INDICATE "indicate"
#define CHARACTERISTIC_FLAG_INDICATE_ENABLED_BIT (1 << 5)
#define CHARACTERISTIC_FLAG_AUTHENTICATED_SIGNED_WRITES "authenticated-signed-writes"
#define CHARACTERISTIC_FLAG_AUTHENTICATED_SIGNED_WRITES_ENABLED_BIT (1 << 6)
#define CHARACTERISTIC_FLAG_EXTENDED_PROPERTIES "extended-properties"
#define CHARACTERISTIC_FLAG_EXTENDED_PROPERTIES_ENABLED_BIT (1 << 7)
#define CHARACTERISTIC_FLAG_RELIABLE_WRITE "reliable-write"
#define CHARACTERISTIC_FLAG_RELIABLE_WRITE_ENABLED_BIT (1 << 8)
#define CHARACTERISTIC_FLAG_WRITABLE_AUXILIARIES "writable-auxiliaries"
#define CHARACTERISTIC_FLAG_WRITABLE_AUXILIARIES_ENABLED_BIT (1 << 9)
#define CHARACTERISTIC_FLAG_ENCRYPTED_READ "encrypt-read"
#define CHARACTERISTIC_FLAG_ENCRYPTED_READ_ENABLED_BIT (1 << 10)
#define CHARACTERISTIC_FLAG_ENCRYPTED_WRITE "encrypt-write"
#define CHARACTERISTIC_FLAG_ENCRYPTED_WRITE_ENABLED_BIT (1 << 11)
#define CHARACTERISTIC_FLAG_ENCRYPTED_AUTHENTICATED_READ "encrypt-authenticated-read"
#define CHARACTERISTIC_FLAG_ENCRYPTED_AUTHENTICATED_READ_ENABLED_BIT (1 << 12)
#define CHARACTERISTIC_FLAG_ENCRYPTED_AUTHENTICATED_WRITE "encrypt-authenticated-write"
#define CHARACTERISTIC_FLAG_ENCRYPTED_AUTHENTICATED_WRITE_ENABLED_BIT (1 << 13)
#define CHARACTERISTIC_FLAG_SECURE_READ "secure-read"
#define CHARACTERISTIC_FLAG_SECURE_READ_ENABLED_BIT (1 << 14)
#define CHARACTERISTIC_FLAG_SECURE_WRITE "secure-write"
#define CHARACTERISTIC_FLAG_SECURE_WRITE_ENABLED_BIT (1 << 15)
#define CHARACTERISTIC_FLAG_AUTHORIZE "authorize"
#define CHARACTERISTIC_FLAG_AUTHORIZE_ENABLED_BIT (1 << 16)

#define DESCRIPTOR_FLAGS_ALL_ENABLED 0x01FF
#define DESCRIPTOR_FLAG_READ "read"
#define DESCRIPTOR_FLAG_READ_ENABLED_BIT (1 << 0)
#define DESCRIPTOR_FLAG_WRITE "write"
#define DESCRIPTOR_FLAG_WRITE_ENABLED_BIT (1 << 1)
#define DESCRIPTOR_FLAG_ENCRYPT_READ "encrypt-read"
#define DESCRIPTOR_FLAG_ENCRYPT_READ_ENABLED_BIT (1 << 2)
#define DESCRIPTOR_FLAG_ENCRYPT_WRITE "encrypt-write"
#define DESCRIPTOR_FLAG_ENCRYPT_WRITE_BIT (1 << 3)
#define DESCRIPTOR_FLAG_ENCRYPT_AUTHENTICATED_READ "encrypt-authenticated-read"
#define DESCRIPTOR_FLAG_ENCRYPT_AUTHENTICATED_READ_ENABLED_BIT (1 << 4)
#define DESCRIPTOR_FLAG_ENCRYTP_AUTHENTICATED_WRITE "encrypt-authenticated-write"
#define DESCRIPTOR_FLAG_ENCRYPT_AUTHENTICATED_WRITE_ENABLED_BIT (1 << 5)
#define DESCRIPTOR_FLAG_SECURE_READ "secure-read"
#define DESCRIPTOR_FLAG_SECURE_READ_ENABLED_BIT (1 << 6)
#define DESCRIPTOR_FLAG_SECURE_WRITE "secure-write"
#define DESCRIPTOR_FLAG_SECURE_WRITE_ENABLED_BIT (1 << 7)
#define DESCRIPTOR_FLAG_AUTHORIZE "authorize"
#define DESCRIPTOR_FLAG_AUTHORIZE_ENABLED_BIT (1 << 8)

#define min(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


//lua userdata object names
#define LUA_USERDATA_DEVICE "device"
#define LUA_USERDATA_SERVICE "service"
#define LUA_USERDATA_CHARACTERISTIC "characteristic"
#define LUA_USERDATA_DESCRIPTOR "descriptor"

#define LUA_INDEX_FIELD "__index"
#define LUA_GARBAGE_COLLECTOR_FIELD "__gc"
//lua api names
#define LUA_API_CREATE_DEVICE "createDevice"
#define LUA_API_CREATE_SERVICE "createService"
#define LUA_API_CREATE_CHARACTERISTIC "createCharacteristic"
#define LUA_API_CREATE_DESCRIPTOR "createDescriptor"
#define LUA_API_REGISTER_DEVICE "registerDevice"

#define LUA_API_FUNCTION_UPDATE "Update"

//lua device methods
#define LUA_DEVICE_ADD_SERVICE "addService"
#define LUA_DEVICE_SET_POWERED "powered"
#define LUA_DEVICE_SET_DISCOVERABLE "discoverable"

//lua service methods
#define LUA_SERVICE_ADD_CHARACTERISTIC "addCharacteristic"

//lua characteristic methods
#define LUA_CHARACTERISTIC_ADD_DESCRIPTOR "addDescriptor"
#define LUA_CHARACTERISTIC_SET_NOTIFYING "notifying"
#define LUA_CHARACTERISTIC_SET_VALUE "setValue"
//lua descriptor methods

#endif //BLE_SIM_DEFINES_H
