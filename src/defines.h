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

#define DEFAULT_CONTROLLER "/org/bluez/hci0"

#define EMPTY_STRING ""

#define DEVICE_OBJECT_NAME "dev"
#define SERVICE_OBJECT_NAME "serv"
#define CHARACTERISTIC_OBJECT_NAME "char"
#define DESCRIPTOR_OBJECT_NAME "desc"

#define BLUEZ_BUS_NAME "org.bluez"
#define BLE_SIM_SERVICE_NAME "org.blesim"

#define DBUS_INTERFACE_OBJECT_MANAGER "org.freedesktop.DBus.ObjectManager"
#define DBUS_METHOD_GET_MANAGED_OBJECTS "GetManagedObjects"

#define BLUEZ_GATT_MANAGER_INTERFACE "org.bluez.GattManager1"
#define BLUEZ_METHOD_REGISTER_APPLICATION "RegisterApplication"

#define BLUEZ_GATT_SERVICE_INTERFACE "org.bluez.GattService1"
#define BLUEZ_GATT_CHARACTERISTIC_INTERFACE "org.bluez.GattCharacteristic1"
#define BLUEZ_GATT_DESCRIPTOR_INTERFACE "org.bluez.GattDescriptor1"

#define DEFAULT_TIMEOUT 1000

#endif //BLE_SIM_DEFINES_H
