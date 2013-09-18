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

static int unpack_rgb_lua(lua_State *L) {
    int c = lua_tointeger(L, 1);
    int r = (c>>16) & 0xFF;
    int g = (c>> 8) & 0xFF;
    int b = (c>> 0) & 0xFF;
    lua_pushinteger(L, r);
    lua_pushinteger(L, g);
    lua_pushinteger(L, b);
    return 3;
}

static int pack_rgb_lua(lua_State *L) {
    int r = lua_tointeger(L, 1);
    int g = lua_tointeger(L, 2);
    int b = lua_tointeger(L, 3);
    int c = (r&0xFF) << 16 | (g&0xFF) <<  8 | (b&0xFF) <<  0;
    lua_pushinteger(L, c);
    return 1;
}

const luaL_Reg color_lib[] = {
    {"scale", scale_color_lua},
    {"pack_rgb", pack_rgb_lua},
    {"unpack_rgb", unpack_rgb_lua},
    {NULL, NULL}
};
