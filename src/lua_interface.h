/***********************************************************************
 *
 * Copyright (c) 2021
 * IoTech Ltd
 *
 **********************************************************************/

#ifndef BLE_SIM_LUA_H
#define BLE_SIM_LUA_H

#include <stdbool.h>

#include "lua5.3/lua.h"
#include "lua5.3/lauxlib.h"
#include "lua5.3/lualib.h"

bool luai_init_state (const char *script_path);

void luai_cleanup (void);

bool luai_call_update (void);


#endif //BLE_SIM_LUA_H
