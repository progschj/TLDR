#include <timelib.h>

#include <GLFW/glfw3.h>

static int time_get_seconds_lua(lua_State *L) {
    lua_pushnumber(L, glfwGetTime());
    return 1;
}

const luaL_Reg time_lib[] = {
    {"seconds", time_get_seconds_lua},
    {NULL, NULL}
};

