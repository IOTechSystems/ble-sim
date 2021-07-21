# BLE Device Simulator
A simulator for ble devices.

# Building

## Dependencies
[D-Bus](https://www.freedesktop.org/wiki/Software/dbus/) 

  `sudo apt-get install libdbus-1-dev` 

[Lua5.3](https://www.lua.org/versions.html)

  `sudo apt install liblua5.3-dev` 

[Bluez 5.60](http://www.bluez.org/)

Some of the Bluez 5.60 source is used to build the simulator so we need to download it:

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

To run the simulator with a lua script, pass use the --script option with the path to the lua script as its argument:

  `./build/realease/ble-sim/ble-sim --script ./path/to/lua/script.lua`

## Running the docker image

After [building](#Building-the-Docker-image) the docker image, run the docker container with the following options:

- **Bind the system bus**
  - `-v /var/run/dbus/system_bus_socket:/var/run/dbus/system_bus_socket`
- **Run the container as privileged**
  - `--privileged`
- **Use the --script option to pass in a lua script to run** 
  - Some example scripts are provided in `./example-scripts/`
  - `--script multiple-devices.lua`

e.g 
 
 ` 
    docker run 
    --privileged 
    -v /var/run/dbus/system_bus_socket:/var/run/dbus/system_bus_socket 
    iotechsys/ble-sim:{VERSION}
    --script ./example-scripts/multiple-devices.lua
  `