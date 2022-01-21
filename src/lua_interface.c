/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "lua_interface.h"
#include "defines.h"
#include "device.h"
#include "utils.h"
#include "logger.h"

#define LUA_API_ENUM_DATATYPE "DataType"

#define LUA_ENUM(L, name, val) \
  lua_pushlstring(L, #name, sizeof(#name)-1); \
  lua_pushnumber(L, val); \
  lua_settable(L, -3);


static lua_State *luai_state;

static void lua_fail (lua_State *lua_state);

static bool init_lua_state (lua_State **lua_state, const char *file_path);

static void luai_setup_lua_sim_api (lua_State *lua_state);

static void luai_register_datatype_enums (lua_State *lua_state);

static void luai_setup_object_metatables (lua_State *lua_state);

static bool luai_call_function (lua_State *lua_state, const char *function_name);

static void luai_check_type (lua_State *lua_state, int index, int expected_parameter_type);

static void luai_check_argument_count (lua_State *lua_state, int expected_argument_count);

static bool luai_get_array (lua_State *lua_state, int idx, ble_data_type_t type, void **array, size_t *array_size);

static bool luai_lua_data_arg_to_ble_data_type (
  lua_State *lua_state,
  int index,
  ble_data_type_t type,
  void **data,
  size_t *data_size
);

//lua api
static int luai_create_device (lua_State *lua_state);

static int luai_create_service (lua_State *lua_state);

static int luai_create_characteristic (lua_State *lua_state);

static int luai_create_descriptor (lua_State *lua_state);

static int luai_register_device (lua_State *lua_state);

//lua device methods
static int luai_device_add_service (lua_State *lua_state);

static int luai_device_set_powered (lua_State *lua_state);

static int luai_device_set_discoverable (lua_State *lua_state);

static int luai_device_free (lua_State *lua_state);

//lua service methods
static int luai_service_add_characteristic (lua_State *lua_state);

static int luai_service_free (lua_State *lua_state);

//lua characteristic methods
static int luai_characteristic_add_descriptor (lua_State *lua_state);

static int luai_characteristic_set_notifying (lua_State *lua_state);

static int luai_characteristic_set_value (lua_State *lua_state);

static int luai_characteristic_free (lua_State *lua_state);

//lua descriptor methods
static int luai_descriptor_free (lua_State *lua_state);

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

static ble_data_type_t luai_check_type_ble_data_type (lua_State *lua_state, int index)
{
  luai_check_type (lua_state, index, LUA_TNUMBER);
  uint8_t type_int = (uint8_t) lua_tointeger(lua_state, index);
  luaL_argcheck (lua_state, type_int <= BLE_STRING, index, "Argument must be a valid" LUA_API_ENUM_DATATYPE " enum: ");
  ble_data_type_t type = (ble_data_type_t) lua_tointeger(lua_state, index);
  return type;
}

static void *luai_check_argument_userdata (
  lua_State *lua_state,
  int index,
  const char *metadata_table_name,
  const char *error_message
)
{
  void *userdata = luaL_checkudata (lua_state, index, metadata_table_name);
  luaL_argcheck(lua_state, userdata != NULL, index, error_message);
  return userdata;
}

static device_t *luai_check_argument_device (lua_State *lua_state, int index) //checks device is the first arguement
{
  return (device_t *) luai_check_argument_userdata (lua_state, index, LUA_USERDATA_DEVICE, "' " LUA_USERDATA_DEVICE "' expected");
}

static service_t *luai_check_argument_service (lua_State *lua_state, int index) //checks device is the first arguement
{
  return (service_t *) luai_check_argument_userdata (lua_state, index, LUA_USERDATA_SERVICE, "' " LUA_USERDATA_SERVICE "' expected");
}

static characteristic_t *luai_check_argument_characteristic (lua_State *lua_state, int index) //checks device is the first arguement
{
  return (characteristic_t *) luai_check_argument_userdata (lua_state, index, LUA_USERDATA_CHARACTERISTIC, "' " LUA_USERDATA_CHARACTERISTIC "' expected");
}

static descriptor_t *luai_check_argument_descriptor (lua_State *lua_state, int index) //checks device is the first arguement
{
  return (descriptor_t *) luai_check_argument_userdata (lua_state, index, LUA_USERDATA_DESCRIPTOR, "' " LUA_USERDATA_DESCRIPTOR "' expected");
}

static bool luai_call_function (lua_State *lua_state, const char *function_name)
{
  lua_getglobal (lua_state, function_name);
  if (lua_pcall (lua_state, 0, 0, 0))
  {
    log_error ("No '%s' function found.\n", function_name);
    lua_fail (lua_state);
    return false;
  }
  return true;
}

static bool init_lua_state (lua_State **lua_state, const char *file_path)
{
  *lua_state = luaL_newstate ();
  luaL_openlibs (*lua_state);

  luai_setup_lua_sim_api (*lua_state);
  luai_setup_object_metatables (*lua_state);

  if (luaL_loadfile (*lua_state, file_path) || lua_pcall (*lua_state, 0, 0, 0))
  {
    lua_fail (*lua_state);
    return false;
  }
  return true;
}

static void luai_setup_object_metatables (lua_State *lua_state)
{
  //device
  luaL_newmetatable (lua_state, LUA_USERDATA_DEVICE);
  lua_pushvalue (lua_state, -1); // there are two 'copies' of the metatable on the stack
  lua_setfield (lua_state, -2, LUA_INDEX_FIELD); // pop one of those copies and assign it to  __index field of the 1st metatable
  luaL_setfuncs (lua_state, luai_device_object_functions, 0);

  lua_pushcfunction (lua_state, luai_device_free); //set garbage collector cleanup function
  lua_setfield (lua_state, -2, LUA_GARBAGE_COLLECTOR_FIELD);

  //service
  luaL_newmetatable (lua_state, LUA_USERDATA_SERVICE);
  lua_pushvalue (lua_state, -1);
  lua_setfield (lua_state, -2, LUA_INDEX_FIELD);
  luaL_setfuncs (lua_state, luai_service_object_functions, 0);

  lua_pushcfunction (lua_state, luai_service_free); //set garbage collector cleanup function
  lua_setfield (lua_state, -2, LUA_GARBAGE_COLLECTOR_FIELD);
  //characterisitc
  luaL_newmetatable (lua_state, LUA_USERDATA_CHARACTERISTIC);
  lua_pushvalue (lua_state, -1);
  lua_setfield (lua_state, -2, LUA_INDEX_FIELD);
  luaL_setfuncs (lua_state, luai_characteristic_object_functions, 0);

  lua_pushcfunction (lua_state, luai_characteristic_free); //set garbage collector cleanup function
  lua_setfield (lua_state, -2, LUA_GARBAGE_COLLECTOR_FIELD);
  //descriptor
  luaL_newmetatable (lua_state, LUA_USERDATA_DESCRIPTOR);
  lua_pushvalue (lua_state, -1);
  lua_setfield (lua_state, -2, LUA_INDEX_FIELD);
  luaL_setfuncs (lua_state, luai_descriptor_object_functions, 0);

  lua_pushcfunction (lua_state, luai_descriptor_free); //set garbage collector cleanup function
  lua_setfield (lua_state, -2, LUA_GARBAGE_COLLECTOR_FIELD);
}

static void luai_register_datatype_enums (lua_State *lua_state)
{
  lua_newtable(lua_state);
  {
    LUA_ENUM(lua_state, BOOL, BLE_BOOL);
    LUA_ENUM(lua_state, INT8, BLE_INT8);
    LUA_ENUM(lua_state, UINT8, BLE_UINT8);
    LUA_ENUM(lua_state, INT16, BLE_INT16);
    LUA_ENUM(lua_state, UINT16, BLE_UINT16);
    LUA_ENUM(lua_state, INT32, BLE_INT32);
    LUA_ENUM(lua_state, UINT32, BLE_UINT32);
    LUA_ENUM(lua_state, INT64, BLE_INT64);
    LUA_ENUM(lua_state, UINT64, BLE_UINT64);
    LUA_ENUM(lua_state, FLOAT, BLE_FLOAT);
    LUA_ENUM(lua_state, DOUBLE, BLE_DOUBLE);
    LUA_ENUM(lua_state, STRING, BLE_STRING);
  }
  lua_setglobal(lua_state, LUA_API_ENUM_DATATYPE);
}

static void luai_setup_lua_sim_api (lua_State *lua_state)
{
  luaL_newlib(lua_state, luai_ble_sim_api);
  lua_setglobal (lua_state, "ble");

  luai_register_datatype_enums (lua_state);
}

static void lua_fail (lua_State *lua_state)
{
  log_error ("Lua - %s", lua_tostring(lua_state, -1));
}

static void luai_check_type (lua_State *lua_state, int index, int expected_parameter_type)
{
  int type = lua_type (lua_state, index);

  luaL_argcheck (
    lua_state,
    type == expected_parameter_type,
    index,
    ""
  );
}

static void luai_check_argument_count (lua_State *lua_state, int expected_argument_count)
{
  if (lua_gettop (lua_state) != expected_argument_count)
  {
    luaL_error(lua_state, "Expected %d arguments to function", expected_argument_count);
  }
}

static int luai_create_device (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 1);
  luai_check_type (lua_state, 1, LUA_TSTRING);
  const char *device_name = lua_tostring(lua_state, 1);

  device_t *device = (device_t *) lua_newuserdata (lua_state, sizeof (*device));
  device_init (device, device_name, ORIGIN_LUA);

  luaL_getmetatable (lua_state, LUA_USERDATA_DEVICE); //add the userdata metatable to this object
  lua_setmetatable (lua_state, -2);

  return 1; //the device is already on the stack so return 
}

static int luai_create_service (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 1);
  luai_check_type (lua_state, 1, LUA_TSTRING);
  const char *uuid = lua_tostring(lua_state, 1);

  service_t *service = (service_t *) lua_newuserdata (lua_state, sizeof (*service));
  service_init (service, uuid, true, ORIGIN_LUA);

  luaL_getmetatable (lua_state, LUA_USERDATA_SERVICE); //add the userdata metatable to this object
  lua_setmetatable (lua_state, -2);

  return 1; //the service is already on the stack so return 
}

static int luai_create_characteristic (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 1);
  luai_check_type (lua_state, 1, LUA_TSTRING);
  const char *char_uuid = lua_tostring(lua_state, 1);

  characteristic_t *characteristic = (characteristic_t *) lua_newuserdata (lua_state, sizeof (*characteristic));
  characteristic_init (characteristic, char_uuid, ORIGIN_LUA);

  luaL_getmetatable (lua_state, LUA_USERDATA_CHARACTERISTIC); //add the userdata metatable to this object
  lua_setmetatable (lua_state, -2);

  return 1; //the characteristic is already on the stack so return this
}

static int luai_create_descriptor (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 1);
  luai_check_type (lua_state, 1, LUA_TSTRING);
  const char *desc_uuid = lua_tostring(lua_state, 1);

  descriptor_t *descriptor = (descriptor_t *) lua_newuserdata (lua_state, sizeof (*descriptor));
  descriptor_init (descriptor, desc_uuid, ORIGIN_LUA);

  luaL_getmetatable (lua_state, LUA_USERDATA_DESCRIPTOR); //add the userdata metatable to this object
  lua_setmetatable (lua_state, -2);

  return 1; //the device is already on the stack so return this
}

static int luai_register_device (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 1);
  device_t *device = luai_check_argument_device (lua_state, 1);

  bool success = device_register (device);
  lua_pushboolean (lua_state, success);
  return 1;
}

static int luai_device_add_service (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 2);
  device_t *device = luai_check_argument_device (lua_state, 1);
  service_t *service = luai_check_argument_service (lua_state, 2);

  bool success = device_add_service (device, service);
  lua_pushboolean (lua_state, success);
  return 1;
}

static int luai_device_set_powered (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 2);
  device_t *device = luai_check_argument_device (lua_state, 1);
  bool powered = lua_toboolean (lua_state, 2);

  bool success = device_set_powered (device, powered);
  lua_pushboolean (lua_state, success);
  return 1;
}

static int luai_device_set_discoverable (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 2);
  device_t *device = luai_check_argument_device (lua_state, 1);
  bool discoverable = lua_toboolean (lua_state, 2);

  bool success = device_set_discoverable (device, discoverable);
  lua_pushboolean (lua_state, success);
  return 1;
}

static int luai_device_free (lua_State *lua_state)
{
  device_t *device = luai_check_argument_device (lua_state, 1);
  device_fini (device);
  return 0;
}

static int luai_service_add_characteristic (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 2);
  service_t *service = luai_check_argument_service (lua_state, 1);
  characteristic_t *characteristic = luai_check_argument_characteristic (lua_state, 2);

  bool success = service_add_characteristic (service, characteristic);
  lua_pushboolean (lua_state, success);
  return 1;
}

static int luai_service_free (lua_State *lua_state)
{
  service_t *service = luai_check_argument_service (lua_state, 1);
  service_fini (service);
  return 0;
}

static int luai_characteristic_add_descriptor (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 2);
  characteristic_t *characteristic = luai_check_argument_characteristic (lua_state, 1);
  descriptor_t *descriptor = luai_check_argument_descriptor (lua_state, 2);

  bool success = characteristic_add_descriptor (characteristic, descriptor);
  lua_pushboolean (lua_state, success);
  return 1;
}

static int luai_characteristic_set_notifying (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 2);
  characteristic_t *characteristic = luai_check_argument_characteristic (lua_state, 1);
  bool notifying = lua_toboolean (lua_state, 2);

  characteristic_set_notifying (characteristic, notifying);
  return 0;
}

static int luai_characteristic_free (lua_State *lua_state)
{
  characteristic_t *characteristic = luai_check_argument_characteristic (lua_state, 1);
  characteristic_fini (characteristic);
  return 0;
}

//TODO: could extract each case into a macro
static bool luai_lua_number_to_opc_ua_data_type (
  lua_State *lua_state,
  int index, ble_data_type_t type,
  void **data,
  size_t *data_size
)
{
  switch (type)
  {
    case BLE_INT8:
    {
      int8_t val = (int8_t) lua_tointeger (lua_state, index);
      *data = malloc (sizeof (int8_t));
      if (*data == NULL)
      {
        return false;
      }
      *data_size = sizeof (int8_t);
      memcpy (*data, &val, sizeof(int8_t));
    }
      break;
    case BLE_UINT8:
    {
      uint8_t val = (uint8_t) lua_tointeger (lua_state, index);
      *data = malloc (sizeof (uint8_t));
      if (*data == NULL)
      {
        return false;
      }
      *data_size = sizeof (uint8_t);
      memcpy (*data, &val, sizeof(uint8_t));
    }
      break;
    case BLE_INT16:
    {
      int16_t val = (int16_t) lua_tointeger (lua_state, index);
      *data = malloc (sizeof (int16_t));
      if (*data == NULL)
      {
        return false;
      }
      *data_size = sizeof (int16_t);
      memcpy (*data, &val, sizeof(int16_t));
    }
      break;
    case BLE_UINT16:
    {
      uint16_t val = (uint16_t) lua_tointeger (lua_state, index);
      *data = malloc (sizeof (uint16_t));
      if (*data == NULL)
      {
        return false;
      }
      *data_size = sizeof (uint16_t);
      memcpy (*data, &val, sizeof(uint16_t));
    }
      break;
    case BLE_INT32:
    {
      int32_t val = (int32_t) lua_tointeger (lua_state, index);
      *data = malloc (sizeof (int8_t));
      if (*data == NULL)
      {
        return false;
      }
      *data_size = sizeof (int32_t);
      memcpy (*data, &val, sizeof(int32_t));
    }
      break;
    case BLE_UINT32:
    {
      uint32_t val = (uint32_t) lua_tointeger (lua_state, index);
      *data = malloc (sizeof (uint32_t));
      if (*data == NULL)
      {
        return false;
      }
      *data_size = sizeof (uint32_t);
      memcpy (*data, &val, sizeof(uint32_t));
    }
      break;
    case BLE_INT64:
    {
      int64_t val = (int64_t) lua_tointeger (lua_state, index);
      *data = malloc (sizeof (int64_t));
      if (*data == NULL)
      {
        return false;
      }
      *data_size = sizeof (int64_t);
      memcpy (*data, &val, sizeof(int64_t));
    }
      break;
    case BLE_UINT64:
    {
      uint64_t val = (uint64_t) lua_tointeger (lua_state, index);
      *data = malloc (sizeof (uint64_t));
      if (*data == NULL)
      {
        return false;
      }
      *data_size = sizeof (uint64_t);
      memcpy (*data, &val, sizeof(uint64_t));
    }
      break;
    case BLE_FLOAT:
    {
      float val = (float) lua_tonumber (lua_state, index);
      *data = malloc (sizeof (float));
      if (*data == NULL)
      {
        return false;
      }
      *data_size = sizeof (float);
      memcpy (*data, &val, sizeof(float));
    }
      break;
    case BLE_DOUBLE:
    {
      double val = (double) lua_tonumber (lua_state, index);
      *data = malloc (sizeof (double));
      if (*data == NULL)
      {
        return false;
      }
      *data_size = sizeof (double);
      memcpy (*data, &val, sizeof(double));
    }
      break;
    default:
      luaL_argerror(lua_state, index, "Argument type must match specified data type");
      return false;
      break;
  }
  return true;
}

static bool luai_lua_data_arg_to_ble_data_type (
  lua_State *lua_state,
  int index,
  ble_data_type_t type,
  void **data,
  size_t *data_size
)
{
  int lua_arg_type = lua_type (lua_state, index);
  switch (lua_arg_type)
  {
    case LUA_TBOOLEAN: {
      if (type != BLE_BOOL)
      {
        return false;
      }
      bool val = lua_toboolean(lua_state, index);
      *data = malloc(sizeof (val));
      if (*data == NULL)
      {
        return false;
      }
      *data_size = sizeof (val);
      memcpy (*data, &val, sizeof (val));
    }
      break;
    case LUA_TNUMBER: {
      bool success = luai_lua_number_to_opc_ua_data_type(lua_state, index, type, data, data_size);
      if (!success)
      {
        return false;
      }
    }
      break;
    case LUA_TSTRING: {
      const char *str = lua_tostring(lua_state, index);
      if (str == NULL) {
        return false;
      }
      *data = strdup(str);
      if (*data == NULL)
      {
        return false;
      }
      *data_size = strlen(str);
    }
    default:
      return false;
  }

  return true;
}

static bool luai_get_array (lua_State *lua_state, int idx, ble_data_type_t type, void **array, size_t *array_size)
{
  size_t items = lua_rawlen (lua_state, idx);
  if (items == 0)
  {
    return false;
  }

  if (type == BLE_STRING)
  {
    luaL_argerror(lua_state, idx, "Argument type cannot be an array of strings");
    return false;
  }

  size_t type_size = BLE_DATA_TYPE_SIZE[type];

  void *buf = calloc(items,type_size);
  if (buf == NULL)
  {
    return false;
  }

  for (size_t i = 1; i <= items; i++)
  {
    lua_pushinteger (lua_state, i); //push index on to stack for gettable function
    lua_gettable (lua_state, idx);

    if (lua_isnil(lua_state, -1)) //is value null then we need to fix the size of the array
    {
      items = i - 1;
      break;
    }

    if (lua_istable(lua_state, -1))
    {
      luaL_argerror(lua_state, idx, "Argument type cannot be an array of arrays");
      free (buf);
      return false;
    }

    void *item_data = NULL;
    size_t item_size = 0;
    bool success = luai_lua_data_arg_to_ble_data_type (lua_state, -1, type, &item_data, &item_size);
    if (!success || item_size != type_size)
    {
      free (buf);
      return false;
    }

    uint8_t *buf_offset = buf + ((i - 1) * type_size);
    memcpy (buf_offset, item_data, type_size); //equivalent to buf[i - 1] = item_data;
    free (item_data);

    lua_pop(lua_state, 1); //pops one value off the top
  }

  *array = buf;
  *array_size = items * type_size;
  return true;
}

static int luai_characteristic_set_value (lua_State *lua_state)
{
  luai_check_argument_count (lua_state, 3);
  characteristic_t *characteristic = luai_check_argument_characteristic (lua_state, 1);
  ble_data_type_t ble_type = luai_check_type_ble_data_type (lua_state, 3);
  bool success = false;
  void *data = NULL;
  size_t data_size = 0;

  if (lua_istable(lua_state,2))
  {
    success = luai_get_array (lua_state, 2, ble_type, &data, &data_size);
  }
  else
  {
    success = luai_lua_data_arg_to_ble_data_type (lua_state, 2, ble_type, &data, &data_size);
  }

  if (success)
  {
    characteristic_update_value (characteristic, data, data_size, global_dbus_connection);
    free (data);
  }

  lua_pushboolean (lua_state, success);
  return 1;
}

static int luai_descriptor_free (lua_State *lua_state)
{
  descriptor_t *descriptor = luai_check_argument_descriptor (lua_state, 1);
  descriptor_fini (descriptor);
  return 0;
}

bool luai_call_update ()
{
  if (NULL == luai_state)
  {
    log_debug ("Lua state was NULL");
    return false;
  }

  return luai_call_function (luai_state, LUA_API_FUNCTION_UPDATE);
}

bool luai_load_script (const char *script_path)
{
  if (!init_lua_state (&luai_state, script_path))
  {
    log_error ("Failed to open luafile");
    return false;
  }

  return true;
}

void luai_cleanup (void)
{
  if (NULL != luai_state)
  {
    lua_close (luai_state);
  }
}
