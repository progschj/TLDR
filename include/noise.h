#ifndef NOISE_H
#define NOISE_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

double noise3(double x, double y, double z);

double noise2(double x, double y);

extern const luaL_Reg noise_lib[];

#endif
