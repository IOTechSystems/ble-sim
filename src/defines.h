/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_DEFINES_H
#define BLE_SIM_DEFINES_H

#define UUID_LENGTH 32
#define DEVICE_PATH_LENGTH 22

#define EMPTY_STRING ""
#define ROOT_PATH "/"
#define DBUS_PROPERTY_NULL { NULL, NULL, NULL }

#define DEVICE_OBJECT_NAME "dev"
#define SERVICE_OBJECT_NAME "serv"
#define CHARACTERISTIC_OBJECT_NAME "char"
#define DESCRIPTOR_OBJECT_NAME "desc"

#define BLE_PROPERTY_UUID "UUID"
#define BLE_PROPERTY_PRIMARY "Primary"
#define BLE_PROPERTY_DEVICE "Device"
#define BLE_PROPERTY_SERVICE "Service"
#define BLE_PROPERTY_CHARACTERISTIC "Characteristic"
#define BLE_PROPERTY_VALUE "Value"
#define BLE_PROPERTY_FLAGS "Flags"

#define BLUEZ_BUS_NAME "org.bluez"
#define BLE_SIM_SERVICE_NAME "org.blesim"

#define DBUS_INTERFACE_PROPERTIES "org.freedesktop.DBus.Properties"
#define DBUS_METHOD_SET "Set"

#define DBUS_INTERFACE_OBJECT_MANAGER "org.freedesktop.DBus.ObjectManager"
#define DBUS_METHOD_GET_MANAGED_OBJECTS "GetManagedObjects"

#define BLUEZ_GATT_MANAGER_INTERFACE "org.bluez.GattManager1"
#define BLUEZ_METHOD_REGISTER_APPLICATION "RegisterApplication"

#define BLUEZ_ADAPTER_INTERFACE "org.bluez.Adapter1"
#define BLUEZ_ADAPTER_PROPERTY_POWERED "Powered"
#define BLUEZ_ADAPTER_PROPERTY_DISCOVERABLE "Discoverable"


#define BLUEZ_GATT_SERVICE_INTERFACE "org.bluez.GattService1"
#define BLUEZ_GATT_CHARACTERISTIC_INTERFACE "org.bluez.GattCharacteristic1"
#define BLUEZ_GATT_DESCRIPTOR_INTERFACE "org.bluez.GattDescriptor1"

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

#endif //BLE_SIM_DEFINES_H
