#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int clamp(int x, int low, int high) {
    if(x<low) return low;
    else if(x>high) return high;
    else return x;
}

void check_userdata_type(lua_State *L, int argn, const char *name) {
    if(lua_gettop(L)<argn) luaL_typerror(L, argn, name);
    if(!lua_isuserdata(L, argn)) luaL_typerror(L, argn, name);
    lua_getmetatable(L, argn);
    lua_getfield(L, -1, "__type");
    lua_pushstring(L, name);
    if(!lua_equal(L, -1, -2)) luaL_typerror(L, argn, name);
    lua_pop(L, 3);
}
