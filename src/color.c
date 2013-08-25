#include <utility.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int scale_color(int color, double factor) {
    int r = color>>16 & 0xFF;
    int g = color>> 8 & 0xFF;
    int b = color>> 0 & 0xFF;
    r = clamp(r*factor, 0, 0xFF);
    g = clamp(g*factor, 0, 0xFF);
    b = clamp(b*factor, 0, 0xFF);
    return r<<16 | g<<8 | b<<0;
}

static int scale_color_lua(lua_State *L) {
    lua_pushinteger(L, scale_color(lua_tointeger(L, 1), lua_tonumber(L, 2)));
    return 1;
}

const luaL_Reg color_lib[] = {
    {"scale", scale_color_lua},
    {NULL, NULL}
};
