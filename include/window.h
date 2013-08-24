#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>

#include <lua.h>

void window_register_lua(lua_State *L, GLFWwindow *window);

#endif
