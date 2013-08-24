#ifndef UTILITY_H
#define UTILITY_H

#include <lua.h>

int clamp(int x, int low, int high);

void check_userdata_type(lua_State *L, int argn, const char *name);

#endif
