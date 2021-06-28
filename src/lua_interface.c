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


static lua_State *lua_state;

static void lua_fail (lua_State *L);

static bool init_lua_state (lua_State **L, const char *file_path);

static void luai_setup_lua_sim_api (lua_State *L);

static void luai_setup_object_metatables (lua_State *L);

static bool luai_call_function (lua_State *L, const char *function_name);

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

static int luai_device_set_powered (lua_State *L);

static int luai_device_set_discoverable (lua_State *L);

static int luai_device_free (lua_State *L);

//lua service methods
static int luai_service_add_characteristic (lua_State *L);

static int luai_service_free (lua_State *L);

//lua characteristic methods
static int luai_characteristic_add_descriptor (lua_State *L);

static int luai_characteristic_set_notifying (lua_State *L);

static int luai_characteristic_set_value (lua_State *L);

static int luai_characteristic_free (lua_State *L);

//lua descriptor methods

static int luai_descriptor_free (lua_State *L);


static const struct luaL_Reg luai_ble_sim_api[] = {
  {LUA_API_CREATE_DEVICE,         luai_create_device},
  {LUA_API_CREATE_SERVICE,        luai_create_service},
  {LUA_API_CREATE_CHARACTERISTIC, luai_create_characteristic},
  {LUA_API_CREATE_DESCRIPTOR,     luai_create_descriptor},
  {LUA_API_REGISTER_DEVICE,       luai_register_device},
  {NULL, NULL}
};

static const struct luaL_Reg luai_device_object_functions[] = {
  {LUA_DEVICE_ADD_SERVICE,      luai_device_add_service},
  {LUA_DEVICE_SET_POWERED,      luai_device_set_powered},
  {LUA_DEVICE_SET_DISCOVERABLE, luai_device_set_discoverable},
  {NULL, NULL}
};

static const struct luaL_Reg luai_service_object_functions[] = {
  {LUA_SERVICE_ADD_CHARACTERISTIC, luai_service_add_characteristic},
  {NULL, NULL}
};

static const struct luaL_Reg luai_characteristic_object_functions[] = {
  {LUA_CHARACTERISTIC_ADD_DESCRIPTOR, luai_characteristic_add_descriptor},
  {LUA_CHARACTERISTIC_SET_NOTIFYING,  luai_characteristic_set_notifying},
  {LUA_CHARACTERISTIC_SET_VALUE,      luai_characteristic_set_value},
  {NULL, NULL}
};

static const struct luaL_Reg luai_descriptor_object_functions[] = {
  {NULL, NULL}
};


static void *luai_check_argument_userdata (
  lua_State *L,
  int index,
  const char *metadata_table_name,
  const char *error_message
)
{
  void *userdata = luaL_checkudata (L, index, metadata_table_name);
  luaL_argcheck(L, userdata != NULL, index, error_message);
  return userdata;
}

static device_t *luai_check_argument_device (lua_State *L, int index) //checks device is the first arguement
{
  return (device_t *) luai_check_argument_userdata (L, index, LUA_USERDATA_DEVICE, "' " LUA_USERDATA_DEVICE "' expected");
}

static service_t *luai_check_argument_service (lua_State *L, int index) //checks device is the first arguement
{
  return (service_t *) luai_check_argument_userdata (L, index, LUA_USERDATA_SERVICE, "' " LUA_USERDATA_SERVICE "' expected");
}

static characteristic_t *luai_check_argument_characteristic (lua_State *L, int index) //checks device is the first arguement
{
  return (characteristic_t *) luai_check_argument_userdata (L, index, LUA_USERDATA_CHARACTERISTIC, "' " LUA_USERDATA_CHARACTERISTIC "' expected");
}

static descriptor_t *luai_check_argument_descriptor (lua_State *L, int index) //checks device is the first arguement
{
  return (descriptor_t *) luai_check_argument_userdata (L, index, LUA_USERDATA_DESCRIPTOR, "' " LUA_USERDATA_DESCRIPTOR "' expected");
}

static bool luai_call_function (lua_State *L, const char *function_name)
{
  lua_getglobal (L, function_name);
  if (lua_pcall (L, 0, 0, 0))
  {
    printf ("No '%s' function found.\n", function_name);
    lua_fail (L);
    return false;
  }
  return true;
}

static bool init_lua_state (lua_State **L, const char *file_path)
{
  *L = luaL_newstate ();
  luaL_openlibs (*L);

  luai_setup_lua_sim_api (*L);
  luai_setup_object_metatables (*L);

  if (luaL_loadfile (*L, file_path) || lua_pcall (*L, 0, 0, 0))
  {
    lua_fail (*L);
    return false;
  }
  return true;
}

static void luai_setup_object_metatables (lua_State *L)
{
  //device
  luaL_newmetatable (L, LUA_USERDATA_DEVICE);
  lua_pushvalue (L, -1); // there are two 'copies' of the metatable on the stack
  lua_setfield (L, -2, LUA_INDEX_FIELD); // pop one of those copies and assign it to  __index field of the 1st metatable
  luaL_setfuncs (L, luai_device_object_functions, 0);

  lua_pushcfunction (L, luai_device_free); //set garbage collector cleanup function
  lua_setfield (L, -2, LUA_GARBAGE_COLLECTOR_FIELD);

  //service
  luaL_newmetatable (L, LUA_USERDATA_SERVICE);
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, LUA_INDEX_FIELD);
  luaL_setfuncs (L, luai_service_object_functions, 0);

  lua_pushcfunction (L, luai_service_free); //set garbage collector cleanup function
  lua_setfield (L, -2, LUA_GARBAGE_COLLECTOR_FIELD);
  //characterisitc
  luaL_newmetatable (L, LUA_USERDATA_CHARACTERISTIC);
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, LUA_INDEX_FIELD);
  luaL_setfuncs (L, luai_characteristic_object_functions, 0);

  lua_pushcfunction (L, luai_characteristic_free); //set garbage collector cleanup function
  lua_setfield (L, -2, LUA_GARBAGE_COLLECTOR_FIELD);
  //descriptor
  luaL_newmetatable (L, LUA_USERDATA_DESCRIPTOR);
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, LUA_INDEX_FIELD);
  luaL_setfuncs (L, luai_descriptor_object_functions, 0);

  lua_pushcfunction (L, luai_descriptor_free); //set garbage collector cleanup function
  lua_setfield (L, -2, LUA_GARBAGE_COLLECTOR_FIELD);
}

static void luai_setup_lua_sim_api (lua_State *L)
{
  luaL_newlib(L, luai_ble_sim_api);
  lua_setglobal (L, "ble");
}

static void lua_fail (lua_State *L)
{
  printf ("LUA ERROR: %s\n", lua_tostring(L, -1));
}

static void luai_check_type (lua_State *L, int index, int expected_parameter_type)
{
  int recieved_parameter_type = lua_type (L, index);
  assert (recieved_parameter_type == expected_parameter_type);
}

static void luai_check_argument_count (lua_State *L, int expected_argument_count)
{
  int recieved_argument_count = lua_gettop (L);
  assert (recieved_argument_count == expected_argument_count);
}

static int luai_create_device (lua_State *L)
{
  luai_check_argument_count (L, 1);
  luai_check_type (L, 1, LUA_TSTRING);
  const char *device_name = lua_tostring(L, 1);

  device_t *device = (device_t *) lua_newuserdata (L, sizeof (*device));
  device_init (device, device_name, ORIGIN_LUA);

  luaL_getmetatable (L, LUA_USERDATA_DEVICE); //add the userdata metatable to this object
  lua_setmetatable (L, -2);

  return 1; //the device is already on the stack so return 
}

static int luai_create_service (lua_State *L)
{
  luai_check_argument_count (L, 2);
  luai_check_type (L, 1, LUA_TSTRING);
  luai_check_type (L, 2, LUA_TBOOLEAN);
  const char *uuid = lua_tostring(L, 1);
  bool primary = lua_toboolean (L, 2);

  service_t *service = (service_t *) lua_newuserdata (L, sizeof (*service));
  service_init (service, uuid, primary, ORIGIN_LUA);

  luaL_getmetatable (L, LUA_USERDATA_SERVICE); //add the userdata metatable to this object
  lua_setmetatable (L, -2);

  return 1; //the service is already on the stack so return 
}

static int luai_create_characteristic (lua_State *L)
{
  luai_check_argument_count (L, 1);
  luai_check_type (L, 1, LUA_TSTRING);
  const char *char_uuid = lua_tostring(L, 1);

  characteristic_t *characteristic = (characteristic_t *) lua_newuserdata (L, sizeof (*characteristic));
  characteristic_init (characteristic, char_uuid, ORIGIN_LUA);

  luaL_getmetatable (L, LUA_USERDATA_CHARACTERISTIC); //add the userdata metatable to this object
  lua_setmetatable (L, -2);

  return 1; //the characteristic is already on the stack so return this
}

static int luai_create_descriptor (lua_State *L)
{
  luai_check_argument_count (L, 1);
  luai_check_type (L, 1, LUA_TSTRING);
  const char *desc_uuid = lua_tostring(L, 1);

  descriptor_t *descriptor = (descriptor_t *) lua_newuserdata (L, sizeof (*descriptor));
  descriptor_init (descriptor, desc_uuid, ORIGIN_LUA);

  luaL_getmetatable (L, LUA_USERDATA_DESCRIPTOR); //add the userdata metatable to this object
  lua_setmetatable (L, -2);

  return 1; //the device is already on the stack so return this
}

static int luai_register_device (lua_State *L)
{
  luai_check_argument_count (L, 1);
  device_t *device = luai_check_argument_device (L, 1);

  bool success = device_register (device);
  lua_pushboolean (L, success);
  return 1;
}

static int luai_device_add_service (lua_State *L)
{
  luai_check_argument_count (L, 2);
  device_t *device = luai_check_argument_device (L, 1);
  service_t *service = luai_check_argument_service (L, 2);

  bool success = device_add_service (device, service);
  lua_pushboolean (L, success);
  return 1;
}

static int luai_device_set_powered (lua_State *L)
{
  luai_check_argument_count (L, 2);
  device_t *device = luai_check_argument_device (L, 1);
  bool powered = lua_toboolean (L, 2);

  bool success = device_set_powered (device, powered);
  lua_pushboolean (L, success);
  return 1;
}

static int luai_device_set_discoverable (lua_State *L)
{
  luai_check_argument_count (L, 2);
  device_t *device = luai_check_argument_device (L, 1);
  bool discoverable = lua_toboolean (L, 2);

  bool success = device_set_discoverable (device, discoverable);
  lua_pushboolean (L, success);
  return 1;
}

static int luai_device_free (lua_State *L)
{
  device_t *device = luai_check_argument_device (L, 1);
  device_free (device);
  return 0;
}

static int luai_service_add_characteristic (lua_State *L)
{
  luai_check_argument_count (L, 2);
  service_t *service = luai_check_argument_service (L, 1);
  characteristic_t *characteristic = luai_check_argument_characteristic (L, 2);

  bool success = service_add_characteristic (service, characteristic);
  lua_pushboolean (L, success);
  return 1;
}

static int luai_service_free (lua_State *L)
{
  service_t *service = luai_check_argument_service (L, 1);
  service_free (service);
  return 0;
}

static int luai_characteristic_add_descriptor (lua_State *L)
{
  luai_check_argument_count (L, 2);
  characteristic_t *characteristic = luai_check_argument_characteristic (L, 1);
  descriptor_t *descriptor = luai_check_argument_descriptor (L, 2);

  bool success = characteristic_add_descriptor (characteristic, descriptor);
  lua_pushboolean (L, success);
  return 1;
}

static int luai_characteristic_set_notifying (lua_State *L)
{
  luai_check_argument_count (L, 2);
  characteristic_t *characteristic = luai_check_argument_characteristic (L, 1);
  bool notifying = lua_toboolean (L, 2);

  characteristic_set_notifying (characteristic, notifying);
  return 0;
}

static int luai_characteristic_free (lua_State *L)
{
  characteristic_t *characteristic = luai_check_argument_characteristic (L, 1);
  characteristic_free (characteristic);
  return 0;
}

static int luai_get_array (lua_State *L, int idx, void **array, unsigned *array_size) //gets array at index idx
{
  unsigned size = lua_rawlen (L, idx);
  uint8_t *buf = malloc (size);

  for (int i = 1; i <= size; i++)
  {
    lua_pushinteger (L, i); //push index on to stack for gettable function
    lua_gettable (L, idx);

    if (lua_isnil(L, -1)) //is value null then we need to fix the size of the array
    {
      size = i - 1;
      break;
    }

    if (!lua_isnumber (L, -1))
    {
      printf ("Array value at index (%u) was not a number", i);
      free (buf);
      return false;
    }
    lua_Integer b = lua_tointeger(L, -1);

    if (b < 0 || b > UINT8_MAX)
    {
      printf ("Array value at index (%u) was too large - should be a byte\n", i);
      free (buf);
      return false;
    }

    buf[i - 1] = b;
    lua_pop(L, 1); //pops one value off the top
  }

  *array = buf;
  *array_size = size;
  return true;
}

static int luai_characteristic_set_value (lua_State *L)
{
  luai_check_argument_count (L, 2);
  characteristic_t *characteristic = luai_check_argument_characteristic (L, 1);

  void *data = NULL;
  unsigned data_size = 0;
  bool should_free = false;
  bool success = true;

  int type = lua_type (L, 2);

  switch (type)
  {
    case LUA_TBOOLEAN:
    {
      bool val = lua_toboolean (L, 2);
      data = &val;
      data_size = sizeof (val);
    }
      break;
    case LUA_TNUMBER:
    {
      unsigned val = lua_tonumber (L, 2);
      data = &val;
      data_size = sizeof (val);
    }
      break;
    case LUA_TTABLE:
    {
      success = luai_get_array (L, 2, &data, &data_size);
      should_free = true;
    }
      break;
    default:
      printf ("Parameter to setValue must be bool, number or byte array!\n");
      success = false;
      break;
  }

  if (success)
  {
    characteristic_update_value (characteristic, data, data_size, global_dbus_connection);
  }

  if (should_free && success) //only free if luai_get_array was called & successful
  {
    free (data);
  }

  lua_pushboolean (L, success);
  return 1;
}

static int luai_descriptor_free (lua_State *L)
{
  descriptor_t *descriptor = luai_check_argument_descriptor (L, 1);
  descriptor_free (descriptor);
  return 0;
}

bool luai_call_update ()
{
  return luai_call_function (lua_state, LUA_API_FUNCTION_UPDATE);
}

bool luai_init_state (const char *script_path)
{
  if (!init_lua_state (&lua_state, script_path))
  {
    printf ("Failed to open luafile\n");
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
