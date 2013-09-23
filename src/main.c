#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <window.h>
#include <utility.h>
#include <image.h>
#include <panel.h>
#include <grid.h>
#include <noise.h>
#include <shadowcast.h>
#include <pathfinding.h>
#include <timelib.h>
#include <color.h>


void error_callback(int error, const char* description) {
    (void) error;
    fputs(description, stderr);
}

int main(int argc, char *argv[]) {
    const char *luafile = argc>1?argv[1]:"init.lua";

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if(glfwInit() == GL_FALSE) {
        fprintf(stderr, "failed to init GLFW");
        return 1;
    }
    glfwSetErrorCallback(error_callback);

    GLFWwindow *window;
    if((window = glfwCreateWindow(640, 480, "", NULL, NULL)) == NULL) {
        fprintf(stderr, "failed to open window");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit()) {
        fprintf(stderr, "failed to init GLEW");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    fprintf(stdout, "%s\n", glGetString(GL_VERSION));

    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    // check required extensions
    assert(GLEW_ARB_shader_objects);
    assert(GLEW_ARB_vertex_shader);
    assert(GLEW_ARB_fragment_shader);
    assert(GLEW_ARB_shading_language_100);
    assert(GLEW_ARB_vertex_buffer_object);

    glEnable(GL_TEXTURE_2D);

    luaL_register(L, "image", image_lib);
    luaL_register(L, "panel", panel_lib);
    luaL_register(L, "grid", grid_lib);
    luaL_register(L, "noise", noise_lib);
    luaL_register(L, "fov", shadowcast_lib);
    luaL_register(L, "path", pathfinding_lib);
    luaL_register(L, "time", time_lib);
    luaL_register(L, "color", color_lib);
    window_register_lua(L, window);

    glDisable(GL_DEPTH_TEST);

    if(luaL_dofile(L, luafile)) {
        fprintf(stderr, "lua: %s\n", lua_tostring(L, -1));
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    lua_close(L);

    return 0;
}
