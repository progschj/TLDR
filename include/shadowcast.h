#ifndef SHADOWCAST_H
#define SHADOWCAST_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <grid.h>

void shadowcast(grid *transparent, grid *light, double x0, double y0, int r);

void attenuated_shadowcast(grid *transparent, grid *light, double x0, double y0, int r, double falloff);

extern const luaL_Reg shadowcast_lib[];

#endif
