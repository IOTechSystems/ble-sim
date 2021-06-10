#!/bin/sh

echo "Creating virtual controllers"
./ble-sim/btvirt -L -l2

echo "Running ble sim"
./ble-sim/ble-sim