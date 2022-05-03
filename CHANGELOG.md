# v1.0.1

- Docker container now logs which bluez is being used
- Added option to run bluez in docker container and ability to record devices with bluez when bluez is containerised
- Updated entrypoint to copy bluetooth.conf to the host machine so that bluez can register dbus
- Fixed logging in entrypoint when RUN_BLUEZ is set
- Refactored free functions into fini to match init functions
- Fixed bug with dockerised sim where characteristic values would not correctly be set
- Added support for different types and arrays of types
- Move lua script examples to xrt-examples
- Improved lua error messages
- Changes in values now produce the correct properties changed signal

# v1.0.2
