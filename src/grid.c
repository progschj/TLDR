#include <grid.h>
#include <utility.h>

#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

grid grid_create(int x0, int y0, int width, int height) {
    grid g = {x0, y0, width, height, malloc(width*height*sizeof(double))};
    return g;
}

grid grid_clone(grid *g0) {
    grid g = {g0->x0, g0->y0, g0->width, g0->height, malloc(g0->width*g0->height*sizeof(double))};
    memcpy(g.data, g0->data, g0->width*g0->height*sizeof(double));
    return g;
}

int grid_contains(grid *g, int x, int y) {
    x -= g->x0; y -= g->y0;
    return x >= 0 && x < g->width && y >= 0 && y < g->height;
}

double grid_get(grid *g, int x, int y) {
    x -= g->x0; y -= g->y0;
    if(x >= 0 && x < g->width && y >= 0 && y < g->height)
        return g->data[x*g->height + y];
    else
        return 0.0;
}

void grid_set(grid *g, int x, int y, double v) {
    x -= g->x0; y -= g->y0;
    if(x >= 0 && x < g->width && y >= 0 && y < g->height)
        g->data[x*g->height + y] = v;
}

void grid_fill(grid *g, int x, int y, int width, int height, double v) {
    x = clamp(x, g->x0, g->x0 + g->width);
    y = clamp(y, g->y0, g->y0 + g->height);
    int xend = clamp(x+width, g->x0, g->x0 + g->width);
    int yend = clamp(y+height, g->y0, g->y0 + g->height);
    for(int i = x;i<xend;++i)
        for(int j = y;j<yend;++j)
            g->data[i*g->height + j] = v;
}

void grid_jitter(grid *g, double amount) {
    for(int x = g->x0;x<g->x0+g->width;++x)
        for(int y = g->y0;y<g->y0+g->height;++y)
            g->data[x*g->height + y] += amount*(2.0*rand()/RAND_MAX-1);
}


void grid_destroy(grid g) {
    free(g.data);
}

static int grid_destroy_lua(lua_State *L) {
    grid_destroy(*((grid*)lua_touserdata(L, 1)));
    return 0;
}

static int grid_size_lua(lua_State *L) {
    if(lua_gettop(L)<1) luaL_typerror(L, 1, "grid");
    grid *g = lua_touserdata(L, 1);
    lua_pushinteger(L, g->width);
    lua_pushinteger(L, g->height);
    return 2;
}

static int grid_offset_lua(lua_State *L) {
    if(lua_gettop(L)<1) luaL_typerror(L, 1, "grid");
    grid *g = lua_touserdata(L, 1);
    lua_pushinteger(L, g->x0);
    lua_pushinteger(L, g->y0);
    return 2;
}

static int grid_set_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) return luaL_typerror(L, 1, "grid");
    if(top<2 || !lua_isnumber(L, 2)) return luaL_typerror(L, 2, "number");
    if(top<3 || !lua_isnumber(L, 3)) return luaL_typerror(L, 3, "number");
    if(top<4 || !lua_isnumber(L, 4)) return luaL_typerror(L, 4, "number");
    grid *g = lua_touserdata(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    int value = lua_tonumber(L, 4);
    grid_set(g, x, y, value);
    return 0;
}

static int grid_fill_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) return luaL_typerror(L, 1, "grid");
    if(top<2 || !lua_isnumber(L, 2)) return luaL_typerror(L, 2, "number");
    if(top>2) {
        if(top<3 || !lua_isnumber(L, 3)) return luaL_typerror(L, 3, "number");
        if(top<4 || !lua_isnumber(L, 4)) return luaL_typerror(L, 4, "number");
        if(top<5 || !lua_isnumber(L, 5)) return luaL_typerror(L, 5, "number");
        if(top<6 || !lua_isnumber(L, 6)) return luaL_typerror(L, 6, "number");
    }
    grid *g = lua_touserdata(L, 1);
    int x = top==2?g->x0:lua_tointeger(L, 2);
    int y = top==2?g->y0:lua_tointeger(L, 3);
    int width = top==2?g->width:lua_tointeger(L, 4);
    int height = top==2?g->height:lua_tointeger(L, 5);
    int value = top==2?lua_tonumber(L, 2):lua_tonumber(L, 6);
    grid_fill(g, x, y, width, height, value);
    return 0;
}

static int grid_jitter_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) return luaL_typerror(L, 1, "grid");
    if(top<2 || !lua_isnumber(L, 2)) return luaL_typerror(L, 2, "number");

    grid *g = lua_touserdata(L, 1);
    double amount = lua_tonumber(L, 2);
    grid_jitter(g, amount);
    return 0;
}

static int grid_get_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) return luaL_typerror(L, 1, "grid");
    if(top<2 || !lua_isnumber(L, 2)) return luaL_typerror(L, 2, "number");
    if(top<3 || !lua_isnumber(L, 3)) return luaL_typerror(L, 3, "number");
    grid *g = lua_touserdata(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    lua_pushnumber(L, grid_get(g, x, y));
    return 1;
}

static int grid_contains_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) return luaL_typerror(L, 1, "grid");
    if(top<2 || !lua_isnumber(L, 2)) return luaL_typerror(L, 2, "number");
    if(top<3 || !lua_isnumber(L, 3)) return luaL_typerror(L, 3, "number");
    grid *g = lua_touserdata(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    lua_pushinteger(L, grid_contains(g, x, y));
    return 1;
}

static void grid_create_metatable_lua(lua_State *L) {
    lua_newtable(L);
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, grid_destroy_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_rawset(L, -3);

    lua_pushstring(L, "__type");
    lua_pushstring(L, "grid");
    lua_rawset(L, -3);

    lua_pushstring(L, "size");
    lua_pushcfunction(L, grid_size_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "offset");
    lua_pushcfunction(L, grid_offset_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "get");
    lua_pushcfunction(L, grid_get_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "set");
    lua_pushcfunction(L, grid_set_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "fill");
    lua_pushcfunction(L, grid_fill_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "jitter");
    lua_pushcfunction(L, grid_jitter_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "contains");
    lua_pushcfunction(L, grid_contains_lua);
    lua_rawset(L, -3);

    lua_setmetatable(L, -2);
}

static int grid_create_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1 || !lua_isnumber(L, 1)) return luaL_typerror(L, 1, "number");
    if(top<2 || !lua_isnumber(L, 2)) return luaL_typerror(L, 2, "number");
    if(top<3 || !lua_isnumber(L, 3)) return luaL_typerror(L, 3, "number");
    if(top<4 || !lua_isnumber(L, 4)) return luaL_typerror(L, 4, "number");
    grid *g = lua_newuserdata(L, sizeof(grid));
    *g = grid_create(lua_tointeger(L, 1), lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
    grid_create_metatable_lua(L);
    return 1;
}

static int grid_clone_lua(lua_State *L) {
    check_userdata_type(L, 1, "grid");
    grid *g0 = lua_touserdata(L, 1);
    grid *g = lua_newuserdata(L, sizeof(grid));
    *g = grid_clone(g0);
    grid_create_metatable_lua(L);
    return 1;
}

const luaL_Reg grid_lib[] = {
    {"create", grid_create_lua},
    {"clone", grid_clone_lua},
    {NULL, NULL}
};
