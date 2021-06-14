# BLE Device Simulator
A bluetooth device simulator.

Currently we use the bluez tool "btvirt" to create a virtual device controller(s), however in future we will create this virtual device controller in the sim.

## Dependencies
- [D-Bus](https://www.freedesktop.org/wiki/Software/dbus/) -
D-Bus is an Inter-Process Communication (IPC) and
Remote Procedure Calling (RPC) mechanism
specifically designed for efficient and easy-to-use
communication between processes running on the same
machine.

## Prerequisites
- [D-Bus](https://www.freedesktop.org/wiki/Software/dbus/) -
D-Bus daemon is required to be running on the host machine.
 
- [BlueZ](http://www.bluez.org/) -
BlueZ Linux module is required to be installed on the host
machine.

### Running locally

  we need to build btvirt (a bluez tool), so we need to build bluez:
  GLib libudev libical
  
  `sudo apt-get install libglib2.0 libudev-dev libical-dev `

  rst2man :
  
  apt-get install python-docutils


## Build

## Run