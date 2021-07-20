# BLE Device Simulator
A simulator for ble devices.

# Building

## Dependencies
[D-Bus](https://www.freedesktop.org/wiki/Software/dbus/) 

  `sudo apt-get install libdbus-1-dev` 

[Lua5.3](https://www.lua.org/versions.html)

  `sudo apt install liblua5.3-dev` 

[Bluez 5.60](http://www.bluez.org/)

Some of the Bluez 5.60 source is used to build the simulator so we need to download it

  `./scripts/download-deps.sh`

## Building the binary

To build the binary, use the build script:

`./scripts/build.sh`

## Building the Docker image

To build the docker image, use the dockerise script:

`./scripts/dockerise.sh`

# Run

## Prerequisites
[D-Bus](https://www.freedesktop.org/wiki/Software/dbus/) -
D-Bus daemon is required to be installed and running on the host machine.

`sudo apt-get install dbus`
 
[BlueZ](http://www.bluez.org/) -
BlueZ Linux module is required to be installed and running on the host
machine.

`sudo apt-get install bluez`

[Lua5.3 interpreter](https://www.lua.org/) - 
The lua 5.3 interpreter is required

`sudo apt-get install lua5.3`

## Run locally

To run the simulator locally build the binary and then run the command:

  `./build/release/ble-sim/ble-sim`

## Running the docker image

After [building](#Building-the-Docker-image) the docker image, run the docker container with the following options:

- **Bind the system bus**
  - `-v /var/run/dbus/system_bus_socket:/var/run/dbus/system_bus_socket`
- **Run the container as privileged**
  - `--privileged`

e.g 
 
 ``` 
    docker run 
    --privileged 
    -v /var/run/dbus/system_bus_socket:/var/run/dbus/system_bus_socket 
    iotechsys/ble-sim:{VERSION}
    [ble-sim arguments]
  ```