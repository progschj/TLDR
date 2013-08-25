#ifndef COLOR_H
#define COLOR_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int scale_color(int color, double factor);

extern const luaL_Reg color_lib[];


#endif
