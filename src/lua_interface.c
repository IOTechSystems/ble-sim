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
static bool init_lua_state (lua_State **L, const char* file_path);
static void luai_setup_lua_sim_api (lua_State *L);
static void luai_setup_object_metatables (lua_State *L);

static void luai_check_type (lua_State *L, int index, int expected_parameter_type);
static void luai_check_argument_count (lua_State *L, int expected_argument_count);

//lua api
static int luai_create_device (lua_State *L);
static int luai_create_service (lua_State *L);
static int luai_create_characteristic (lua_State *L);
static int luai_create_descriptor (lua_State *L);
static int luai_register_device (lua_State *L);

//lua device methods
static int luai_device_add_service (lua_State *L);

//lua service methods
static int luai_service_add_characteristic (lua_State *L);

//lua characteristic methods
static int luai_characteristic_add_descriptor (lua_State *L);

//lua descriptor methods

static const struct luaL_Reg luai_ble_sim_api [] = {
  {LUA_API_CREATE_DEVICE, luai_create_device},
  {LUA_API_CREATE_SERVICE, luai_create_service},
  {LUA_API_CREATE_CHARACTERISTIC, luai_create_characteristic},
  {LUA_API_CREATE_DESCRIPTOR, luai_create_descriptor},
  {LUA_API_REGISTER_DEVICE, luai_register_device},
  {NULL, NULL} 
};

static const struct luaL_Reg luai_device_object_functions [] = {
  {LUA_DEVICE_ADD_SERVICE, luai_device_add_service},
  {NULL, NULL}
};

static const struct luaL_Reg luai_service_object_functions [] = {
  {LUA_SERVICE_ADD_CHARACTERISTIC, luai_service_add_characteristic},
  {NULL, NULL}
};

static const struct luaL_Reg luai_characteristic_object_functions [] = {
  {LUA_CHARACTERISTIC_ADD_DESCRIPTOR, luai_characteristic_add_descriptor},
  {NULL, NULL}
};

static const struct luaL_Reg luai_descriptor_object_functions [] = {
  {NULL, NULL}
};


static void *luai_check_argument_userdata(
  lua_State *L,
  int index, 
  const char *metadata_table_name,
  const char *error_message
  )
{
  void *userdata = luaL_checkudata(L, index, metadata_table_name);
  luaL_argcheck(L, userdata != NULL, index,  error_message);
  return userdata;
}

static device_t *luai_check_argument_device(lua_State *L, int index) //checks device is the first arguement
{
  return (device_t *) luai_check_argument_userdata(L, index, LUA_USERDATA_DEVICE, "' " LUA_USERDATA_DEVICE "' expected");
}

static service_t *luai_check_argument_service(lua_State *L, int index) //checks device is the first arguement
{
  return (service_t *) luai_check_argument_userdata(L, index, LUA_USERDATA_SERVICE, "' " LUA_USERDATA_SERVICE "' expected");
}

static characteristic_t *luai_check_argument_characteristic(lua_State *L, int index) //checks device is the first arguement
{
  return (characteristic_t *) luai_check_argument_userdata(L, index, LUA_USERDATA_CHARACTERISTIC, "' " LUA_USERDATA_CHARACTERISTIC "' expected");
}

static descriptor_t *luai_check_argument_descriptor(lua_State *L, int index) //checks device is the first arguement
{
  return (descriptor_t *) luai_check_argument_userdata(L, index, LUA_USERDATA_DESCRIPTOR, "' " LUA_USERDATA_DESCRIPTOR "' expected");
}

static lua_State *lua_state;

static bool init_lua_state(lua_State **L, const char* file_path)
{
  *L = luaL_newstate();
  luaL_openlibs (*L);
  
  luai_setup_lua_sim_api(*L);
  luai_setup_object_metatables(*L);

  if (luaL_loadfile (*L, file_path) || lua_pcall (*L, 0, 0, 0))
  {
    lua_fail(*L);
    return false;
  }
  return true;
}

static void luai_setup_object_metatables (lua_State *L)
{
  //device
  luaL_newmetatable (L, LUA_USERDATA_DEVICE);  
  lua_pushvalue(L, -1); // there are two 'copies' of the metatable on the stack
  lua_setfield(L, -2, LUA_INDEX_FIELD); // pop one of those copies and assign it to  __index field of the 1st metatable
  luaL_setfuncs (L, luai_device_object_functions, 0);
  //service
  luaL_newmetatable (L, LUA_USERDATA_SERVICE);  
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_INDEX_FIELD);               
  luaL_setfuncs (L, luai_service_object_functions, 0);
  //characterisitc
  luaL_newmetatable (L, LUA_USERDATA_CHARACTERISTIC);  
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_INDEX_FIELD);               
  luaL_setfuncs (L, luai_characteristic_object_functions, 0);
  //descriptor
  luaL_newmetatable (L, LUA_USERDATA_DESCRIPTOR);  
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, LUA_INDEX_FIELD);               
  luaL_setfuncs (L, luai_descriptor_object_functions, 0);
}

static void luai_setup_lua_sim_api(lua_State *L)
{
  luaL_newlib(L, luai_ble_sim_api);
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

  device_t *device = (device_t *) lua_newuserdata (L, sizeof(*device));
  device_init (device, device_name);

  luaL_getmetatable (L, LUA_USERDATA_DEVICE); //add the userdata metatable to this object
  lua_setmetatable(L, -2);

  return 1; //the device is already on the stack so return 
}

static int luai_create_service(lua_State *L)
{
  luai_check_argument_count(L, 2);
  luai_check_type (L, 1, LUA_TSTRING);
  luai_check_type (L, 2, LUA_TBOOLEAN);
  const char *uuid = lua_tostring(L, 1);
  bool primary = lua_toboolean (L, 1);

  service_t *service = (service_t *) lua_newuserdata (L, sizeof(*service));
  service_init (service, uuid, primary);

  luaL_getmetatable (L, LUA_USERDATA_SERVICE); //add the userdata metatable to this object
  lua_setmetatable(L, -2);

  return 1; //the service is already on the stack so return 
}

static int luai_create_characteristic(lua_State *L)
{
  luai_check_argument_count(L, 1);
  luai_check_type (L, 1, LUA_TSTRING);
  const char *char_uuid = lua_tostring(L, 1);

  characteristic_t *characteristic = (characteristic_t *) lua_newuserdata (L, sizeof(*characteristic));
  characteristic_init (characteristic, char_uuid);

  luaL_getmetatable (L, LUA_USERDATA_CHARACTERISTIC); //add the userdata metatable to this object
  lua_setmetatable(L, -2);

  return 1; //the characteristic is already on the stack so return this
}

static int luai_create_descriptor(lua_State *L)
{
  luai_check_argument_count(L, 1);
  luai_check_type (L, 1, LUA_TSTRING);
  const char *desc_uuid = lua_tostring(L, 1);

  descriptor_t *descriptor = (descriptor_t *) lua_newuserdata (L, sizeof(*descriptor));
  descriptor_init (descriptor, desc_uuid);

  luaL_getmetatable (L, LUA_USERDATA_DESCRIPTOR); //add the userdata metatable to this object
  lua_setmetatable(L, -2);

  return 1; //the device is already on the stack so return this
}

static int luai_register_device (lua_State *L)
{
  luai_check_argument_count(L, 1);
  device_t *device = luai_check_argument_device (L, 1);

  bool success = device_register(device);
  lua_pushboolean (L, success);
  return 1;
}

static int luai_device_add_service (lua_State *L)
{
  luai_check_argument_count(L, 2);
  device_t *device = luai_check_argument_device (L, 1);
  service_t *service = luai_check_argument_service (L, 2);

  bool success = device_add_service(device, service);
  lua_pushboolean (L, success);
  return 1;
}

static int luai_service_add_characteristic (lua_State *L)
{
  luai_check_argument_count(L, 2);
  service_t *service = luai_check_argument_service (L, 1);
  characteristic_t *characteristic = luai_check_argument_characteristic (L, 2);

  bool success = service_add_characteristic (service, characteristic);
  lua_pushboolean (L, success);
  return 1;
}

static int luai_characteristic_add_descriptor (lua_State *L)
{
  luai_check_argument_count(L, 2);
  characteristic_t *characteristic = luai_check_argument_characteristic (L, 1);
  descriptor_t *descriptor = luai_check_argument_descriptor (L, 2);

  bool success = characteristic_add_descriptor (characteristic, descriptor);
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
