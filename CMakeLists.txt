cmake_minimum_required(VERSION 3.10)

project(ble-sim LANGUAGES C)

# Package support
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(Libdbus REQUIRED)
if (NOT LIBDBUS_FOUND)
  message (WARNING "D-Bus library or header not found")
endif ()

find_library(LUA_LIBRARIES NAMES lua5.3)

set(LUA_INCLUDE_DIR /usr/include/lua5.3/)

# bluez source we need to compile with
set(BLUEZ_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/bluez)

set(BLUEZ_INCLUDES 
  ${CMAKE_SOURCE_DIR}
  ${BLUEZ_INCLUDE_DIR}
  )

file(GLOB BLUEZ_C_FILES
  ${BLUEZ_INCLUDE_DIR}/lib/bluetooth.c 
  ${BLUEZ_INCLUDE_DIR}/src/shared/mainloop.c 
  ${BLUEZ_INCLUDE_DIR}/src/shared/mainloop-notify.c
  ${BLUEZ_INCLUDE_DIR}/src/shared/timeout-mainloop.c
  ${BLUEZ_INCLUDE_DIR}/src/shared/io-mainloop.c
  ${BLUEZ_INCLUDE_DIR}/src/shared/util.c
  ${BLUEZ_INCLUDE_DIR}/src/shared/queue.c
  ${BLUEZ_INCLUDE_DIR}/src/shared/crypto.c
  ${BLUEZ_INCLUDE_DIR}/emulator/vhci.c
  ${BLUEZ_INCLUDE_DIR}/emulator/btdev.c
)

include_directories (
  ${BLUEZ_INCLUDES}
  ${LUA_INCLUDE_DIR}
  ${DBUS_INCLUDE_DIR}
  ${DBUS_ARCH_INCLUDE_DIR}
  src/
)

if (CMAKE_C_COMPILER_ID MATCHES GNU)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wstrict-prototypes -Wmissing-prototypes")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -O0")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -ggdb -O0")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -O3")
endif()

file(GLOB C_FILES src/*.c)
add_executable(ble-sim ${C_FILES} ${BLUEZ_C_FILES})

FILE(STRINGS "VERSION" VERSION_NUMBER)
target_compile_definitions(ble-sim PRIVATE VERSION="${VERSION_NUMBER}")

target_link_libraries(ble-sim PUBLIC ${DBUS_LIBRARIES} ${LUA_LIBRARIES})
