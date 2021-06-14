#!/bin/sh

#start dbus
rc-status #need this to start dbus and bluetooth
rc-service dbus start
rc-service bluetooth start

# echo "Creating virtual controllers"
# ./ble-sim/btvirt -L -l2

echo "Starting ble Sim"
sleep 5 #wait for dbus to start

echo "Running ble sim"
./ble-sim/ble-sim