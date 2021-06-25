/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/
#include <stdbool.h>
#include <assert.h>

#include "lua_interface.h"
#include "defines.h"
#include "device.h"


static void lua_fail (lua_State *L);
static bool init_lua_state(lua_State **L, const char* file_path);
static void setup_lua_callbacks(lua_State *L);
static void luai_check_type (lua_State *L, int index, int expected_parameter_type);
static void luai_check_argument_count (lua_State *L, int expected_argument_count);

//lua api
static int luai_create_device(lua_State *L);
static int luai_device_add_service(lua_State *L);
static int luai_device_add_characteristic(lua_State *L);
static int luai_device_add_descriptor(lua_State *L);
static int luai_simulator_add_device(lua_State *L);

static lua_State *lua_state;

static bool init_lua_state(lua_State **L, const char* file_path)
{
  *L = luaL_newstate();
  luaL_openlibs (*L);
  
  setup_lua_callbacks(*L);

  if (luaL_loadfile (*L, file_path) || lua_pcall (*L, 0, 0, 0))
  {
    lua_fail(*L);
    return false;
  }
  return true;
}

static void setup_lua_callbacks(lua_State *L)
{
  static const struct luaL_Reg functions [] = {
    {LUA_CALLBACK_CREATE_DEVICE, luai_create_device},
    {LUA_CALLBACK_DEVICE_ADD_SERVICE, luai_device_add_service},
    {LUA_CALLBACK_DEVICE_ADD_CHARACTERISTIC, luai_device_add_characteristic},
    {LUA_CALLBACK_DEVICE_ADD_DESCRIPTOR, luai_device_add_descriptor},
    {LUA_CALLBACK_SIMULATOR_ADD_DEVICE, luai_simulator_add_device},
    {NULL, NULL} 
  };

  luaL_newlib(L, functions);
  lua_setglobal(L, "ble");
}

static void lua_fail (lua_State *L)
{
  printf ("LUA ERROR: %s\n", lua_tostring(L, -1));  
}

static void luai_check_type (lua_State *L, int index, int expected_parameter_type)
{
  int recieved_parameter_type = lua_type(L, index);
  assert (recieved_parameter_type == expected_parameter_type);
}

static void luai_check_argument_count (lua_State *L, int expected_argument_count)
{
  int recieved_argument_count = lua_gettop (L);
  assert (recieved_argument_count == expected_argument_count);
}

static int luai_create_device(lua_State *L)
{
  luai_check_argument_count(L, 1);
  luai_check_type (L, 1, LUA_TSTRING);
  const char *device_name = lua_tostring(L, 1);
  device_t *device = device_new (device_name);
  lua_pushlightuserdata(L, device);
  return 1;
}

static int luai_device_add_service(lua_State *L)
{
  luai_check_argument_count(L, 3);
  luai_check_type (L, 1, LUA_TLIGHTUSERDATA);
  luai_check_type (L, 2, LUA_TSTRING);
  luai_check_type (L, 3, LUA_TBOOLEAN);

  device_t *device = (device_t*) lua_touserdata(L, 1);
  const char *service_uuid = lua_tostring(L, 2);
  bool primary = lua_toboolean(L, 3);

  service_t *service = service_new (service_uuid, primary);
  bool success = device_add_service (device, service);
  if (!success)
  {
    service_free (service);
  }

  lua_pushboolean (L, success);
  return 1;
}

static int luai_device_add_characteristic(lua_State *L)
{
  luai_check_argument_count(L, 3);
  luai_check_type (L, 1, LUA_TLIGHTUSERDATA);
  luai_check_type (L, 2, LUA_TSTRING);
  luai_check_type (L, 3, LUA_TSTRING);

  device_t *device = (device_t*) lua_touserdata(L, 1);
  const char *service_uuid = lua_tostring(L, 2);
  const char *characteristic_uuid = lua_tostring(L, 3);

  characteristic_t *characteristic = characteristic_new (characteristic_uuid);
  bool success = device_add_characteristic (device, service_uuid, characteristic);
  if (!success)
  {
    characteristic_free (characteristic);
  }
  lua_pushboolean (L, success);
  return 1;
}

static int luai_device_add_descriptor(lua_State *L)
{
  luai_check_argument_count(L, 4);
  luai_check_type (L, 1, LUA_TLIGHTUSERDATA);
  luai_check_type (L, 2, LUA_TSTRING);
  luai_check_type (L, 3, LUA_TSTRING);
  luai_check_type (L, 4, LUA_TSTRING);

  device_t *device = (device_t*) lua_touserdata(L, 1);
  const char *service_uuid = lua_tostring(L, 2);
  const char *characteristic_uuid = lua_tostring(L, 3);
  const char *descriptor_uuid = lua_tostring(L, 4);

  //create descriptor
  descriptor_t *descriptor = descriptor_new (descriptor_uuid);
  bool success = device_add_descriptor(device, service_uuid, characteristic_uuid, descriptor);
  if (!success)
  {
    descriptor_free (descriptor);
  }
  lua_pushboolean (L, success);
  return 1;
}

static int luai_simulator_add_device(lua_State *L)
{
  luai_check_argument_count(L, 1);
  luai_check_type (L, 1, LUA_TLIGHTUSERDATA);
  device_t *device = (device_t*) lua_touserdata(L, 1);
  bool success = device_add (device);
  lua_pushboolean (L, success);
  return 1;
}

bool luai_init_state(const char *script_path)
{
  if (!init_lua_state (&lua_state, script_path))
  {
    printf("Failed to open luafile\n");
    return false;
  }

  return true;
}

void luai_cleanup (void)
{
  if (NULL != lua_state)
  {
    lua_close (lua_state);
  }
}
