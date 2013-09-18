#include <GLFW/glfw3.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <utility.h>

typedef struct callbacks_t {
    lua_State *L;
    int mouse_button;
    int mouse_move;
    int scroll;
    int key;
    int character;
    int close;
} callbacks;

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    callbacks *c = glfwGetWindowUserPointer(window);
    if(c->mouse_button == LUA_NOREF) return;
    lua_rawgeti(c->L, LUA_REGISTRYINDEX, c->mouse_button);
    lua_pushinteger(c->L, button);
    lua_pushinteger(c->L, action);
    lua_pushinteger(c->L, mods);
    lua_call(c->L, 3, 0);
}

static void mouse_move_callback(GLFWwindow *window, double x, double y) {
    callbacks *c = glfwGetWindowUserPointer(window);
    if(c->mouse_move == LUA_NOREF) return;
    lua_rawgeti(c->L, LUA_REGISTRYINDEX, c->mouse_move);
    lua_pushnumber(c->L, x);
    lua_pushnumber(c->L, y);
    lua_call(c->L, 2, 0);
}

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    callbacks *c = glfwGetWindowUserPointer(window);
    if(c->scroll == LUA_NOREF) return;
    lua_rawgeti(c->L, LUA_REGISTRYINDEX, c->scroll);
    lua_pushnumber(c->L, xoffset);
    lua_pushnumber(c->L, yoffset);
    lua_call(c->L, 2, 0);

}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    callbacks *c = glfwGetWindowUserPointer(window);
    if(c->key == LUA_NOREF) return;
    lua_rawgeti(c->L, LUA_REGISTRYINDEX, c->key);
    lua_pushinteger(c->L, key);
    lua_pushinteger(c->L, scancode);
    lua_pushinteger(c->L, action);
    lua_pushinteger(c->L, mods);
    lua_call(c->L, 4, 0);
}

static void character_callback(GLFWwindow *window, unsigned int character) {
    callbacks *c = glfwGetWindowUserPointer(window);
    if(c->character == LUA_NOREF) return;
    lua_rawgeti(c->L, LUA_REGISTRYINDEX, c->character);
    lua_pushinteger(c->L, character);
    lua_call(c->L, 1, 0);
}

static void close_callback(GLFWwindow *window) {
    callbacks *c = glfwGetWindowUserPointer(window);
    if(c->close == LUA_NOREF) return;
    lua_rawgeti(c->L, LUA_REGISTRYINDEX, c->close);
    lua_call(c->L, 0, 0);
}

static int window_get_mouse_position_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
}

static int window_get_mouse_button_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    if(top<2) typerror(L, 2, "number");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    lua_pushinteger(L, glfwGetMouseButton(window, lua_tointeger(L, 2)));
    return 1;
}

static int window_get_key_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    if(top<2) typerror(L, 2, "number");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    lua_pushinteger(L, glfwGetKey(window, lua_tointeger(L, 2)));
    return 1;
}

static int window_swap_buffers_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    glfwSwapBuffers(window);
    return 0;
}

static int window_poll_events_lua(lua_State *L) {
    (void) L;
    glfwPollEvents();
    return 0;
}

static int window_wait_events_lua(lua_State *L) {
    (void) L;
    glfwWaitEvents();
    return 0;
}


static int window_size_lua(lua_State *L) {
    if(lua_gettop(L)<1) typerror(L, 1, "window");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int window_resize_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    if(top<3) typerror(L, top+1, "number");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    int width = lua_tointeger(L, 2);
    int height = lua_tointeger(L, 3);
    glfwSetWindowSize(window, width, height);
    glViewport(0, 0, width, height);
    return 0;
}

static int window_settitle_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    if(top<2) typerror(L, top+1, "string");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    const char *title = lua_tostring(L, 2);
    glfwSetWindowTitle(window, title);
    return 0;
}

static int window_key_callback_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    if(top<2) typerror(L, 2, "function");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    lua_getfield(L, 1, "__callbacks");
    callbacks *c = lua_touserdata(L, -1);
    lua_pop(L, 1);
    if(lua_isnil(L, 2)) {
        luaL_unref(L, LUA_REGISTRYINDEX, c->key);
        glfwSetKeyCallback(window, NULL);
        c->key = LUA_NOREF;
    } else if(lua_isfunction(L, 2)) {
        glfwSetKeyCallback(window, key_callback);
        lua_pushvalue(L, 2);
        c->key = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        typerror(L, 2, "function");
    }
    return 0;
}

static int window_character_callback_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    if(top<2) typerror(L, 2, "function");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    lua_getfield(L, 1, "__callbacks");
    callbacks *c = lua_touserdata(L, -1);
    lua_pop(L, 1);
    if(lua_isnil(L, 2)) {
        luaL_unref(L, LUA_REGISTRYINDEX, c->character);
        glfwSetCharCallback(window, NULL);
        c->character = LUA_NOREF;
    } else if(lua_isfunction(L, 2)) {
        glfwSetCharCallback(window, character_callback);
        lua_pushvalue(L, 2);
        c->character = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        typerror(L, 2, "function");
    }
    return 0;
}

static int window_mouse_button_callback_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    if(top<2) typerror(L, 2, "function");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    lua_getfield(L, 1, "__callbacks");
    callbacks *c = lua_touserdata(L, -1);
    lua_pop(L, 1);
    if(lua_isnil(L, 2)) {
        luaL_unref(L, LUA_REGISTRYINDEX, c->mouse_button);
        glfwSetMouseButtonCallback(window, NULL);
        c->mouse_button = LUA_NOREF;
    } else if(lua_isfunction(L, 2)) {
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        lua_pushvalue(L, 2);
        c->mouse_button = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        typerror(L, 2, "function");
    }
    return 0;
}

static int window_mouse_move_callback_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    if(top<2) typerror(L, 2, "function");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    lua_getfield(L, 1, "__callbacks");
    callbacks *c = lua_touserdata(L, -1);
    lua_pop(L, 1);
    if(lua_isnil(L, 2)) {
        luaL_unref(L, LUA_REGISTRYINDEX, c->mouse_move);
        glfwSetCursorPosCallback(window, NULL);
        c->mouse_move = LUA_NOREF;
    } else if(lua_isfunction(L, 2)) {
        glfwSetCursorPosCallback(window, mouse_move_callback);
        lua_pushvalue(L, 2);
        c->mouse_move = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        typerror(L, 2, "function");
    }
    return 0;
}

static int window_scroll_callback_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    if(top<2) typerror(L, 2, "function");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    lua_getfield(L, 1, "__callbacks");
    callbacks *c = lua_touserdata(L, -1);
    lua_pop(L, 1);
    if(lua_isnil(L, 2)) {
        luaL_unref(L, LUA_REGISTRYINDEX, c->scroll);
        glfwSetScrollCallback(window, NULL);
        c->scroll = LUA_NOREF;
    } else if(lua_isfunction(L, 2)) {
        glfwSetScrollCallback(window, scroll_callback);
        lua_pushvalue(L, 2);
        c->scroll = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        typerror(L, 2, "function");
    }
    return 0;
}

static int window_close_callback_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "window");
    if(top<2) typerror(L, 2, "function");
    GLFWwindow *window = *((GLFWwindow**)lua_touserdata(L, 1));
    lua_getfield(L, 1, "__callbacks");
    callbacks *c = lua_touserdata(L, -1);
    lua_pop(L, 1);
    if(lua_isnil(L, 2)) {
        luaL_unref(L, LUA_REGISTRYINDEX, c->close);
        glfwSetWindowCloseCallback(window, NULL);
        c->close = LUA_NOREF;
    } else if(lua_isfunction(L, 2)) {
        glfwSetWindowCloseCallback(window, close_callback);
        lua_pushvalue(L, 2);
        c->close = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        typerror(L, 2, "function");
    }
    return 0;
}

void window_register_lua(lua_State *L, GLFWwindow *window) {
    GLFWwindow **userdata_window = lua_newuserdata(L, sizeof(GLFWwindow*));
    *userdata_window = window;
    lua_newtable(L);

    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_rawset(L, -3);

    lua_pushstring(L, "__callbacks");
    callbacks *c = lua_newuserdata(L, sizeof(callbacks));
    c->L = L;
    c->mouse_button = LUA_NOREF;
    c->mouse_move = LUA_NOREF;
    c->scroll = LUA_NOREF;
    c->key = LUA_NOREF;
    c->character = LUA_NOREF;
    c->close = LUA_NOREF;
    lua_rawset(L, -3);
    glfwSetWindowUserPointer(window, c);

    lua_pushstring(L, "__type");
    lua_pushstring(L, "window");
    lua_rawset(L, -3);

    lua_pushstring(L, "get_mouse_position");
    lua_pushcfunction(L, window_get_mouse_position_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "get_mouse_button");
    lua_pushcfunction(L, window_get_mouse_button_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "get_key");
    lua_pushcfunction(L, window_get_key_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "size");
    lua_pushcfunction(L, window_size_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "resize");
    lua_pushcfunction(L, window_resize_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "set_title");
    lua_pushcfunction(L, window_settitle_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "set_key_callback");
    lua_pushcfunction(L, window_key_callback_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "set_character_callback");
    lua_pushcfunction(L, window_character_callback_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "set_mouse_button_callback");
    lua_pushcfunction(L, window_mouse_button_callback_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "set_mouse_move_callback");
    lua_pushcfunction(L, window_mouse_move_callback_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "set_scroll_callback");
    lua_pushcfunction(L, window_scroll_callback_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "set_close_callback");
    lua_pushcfunction(L, window_close_callback_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "swap_buffers");
    lua_pushcfunction(L, window_swap_buffers_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "poll_events");
    lua_pushcfunction(L, window_poll_events_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "wait_events");
    lua_pushcfunction(L, window_wait_events_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "input");
    lua_newtable(L);

    lua_pushstring(L, "MOUSE_BUTTON_1");
    lua_pushinteger(L, GLFW_MOUSE_BUTTON_1);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOUSE_BUTTON_2");
    lua_pushinteger(L, GLFW_MOUSE_BUTTON_2);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOUSE_BUTTON_3");
    lua_pushinteger(L, GLFW_MOUSE_BUTTON_3);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOUSE_BUTTON_RIGHT");
    lua_pushinteger(L, GLFW_MOUSE_BUTTON_RIGHT);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOUSE_BUTTON_MIDDLE");
    lua_pushinteger(L, GLFW_MOUSE_BUTTON_MIDDLE);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOUSE_BUTTON_LEFT");
    lua_pushinteger(L, GLFW_MOUSE_BUTTON_LEFT);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOUSE_BUTTON_4");
    lua_pushinteger(L, GLFW_MOUSE_BUTTON_4);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOUSE_BUTTON_5");
    lua_pushinteger(L, GLFW_MOUSE_BUTTON_5);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOUSE_BUTTON_6");
    lua_pushinteger(L, GLFW_MOUSE_BUTTON_6);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOUSE_BUTTON_7");
    lua_pushinteger(L, GLFW_MOUSE_BUTTON_7);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOUSE_BUTTON_8");
    lua_pushinteger(L, GLFW_MOUSE_BUTTON_8);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOD_SHIFT");
    lua_pushinteger(L, GLFW_MOD_SHIFT);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOD_ALT");
    lua_pushinteger(L, GLFW_MOD_ALT);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOD_CONTROL");
    lua_pushinteger(L, GLFW_MOD_CONTROL);
    lua_rawset(L, -3);

    lua_pushstring(L, "MOD_SUPER");
    lua_pushinteger(L, GLFW_MOD_SUPER);
    lua_rawset(L, -3);

    lua_pushstring(L, "PRESS");
    lua_pushinteger(L, GLFW_PRESS);
    lua_rawset(L, -3);

    lua_pushstring(L, "RELEASE");
    lua_pushinteger(L, GLFW_RELEASE);
    lua_rawset(L, -3);

    lua_pushstring(L, "REPEAT");
    lua_pushinteger(L, GLFW_REPEAT);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_UNKNOWN");
    lua_pushinteger(L, GLFW_KEY_UNKNOWN);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_SPACE");
    lua_pushinteger(L, GLFW_KEY_SPACE);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_APOSTROPHE");
    lua_pushinteger(L, GLFW_KEY_APOSTROPHE);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_COMMA");
    lua_pushinteger(L, GLFW_KEY_COMMA);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_MINUS");
    lua_pushinteger(L, GLFW_KEY_MINUS);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_PERIOD");
    lua_pushinteger(L, GLFW_KEY_PERIOD);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_SLASH");
    lua_pushinteger(L, GLFW_KEY_SLASH);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_0");
    lua_pushinteger(L, GLFW_KEY_0);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_1");
    lua_pushinteger(L, GLFW_KEY_1);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_2");
    lua_pushinteger(L, GLFW_KEY_2);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_3");
    lua_pushinteger(L, GLFW_KEY_3);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_4");
    lua_pushinteger(L, GLFW_KEY_4);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_5");
    lua_pushinteger(L, GLFW_KEY_5);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_6");
    lua_pushinteger(L, GLFW_KEY_6);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_7");
    lua_pushinteger(L, GLFW_KEY_7);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_8");
    lua_pushinteger(L, GLFW_KEY_8);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_9");
    lua_pushinteger(L, GLFW_KEY_9);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_SEMICOLON");
    lua_pushinteger(L, GLFW_KEY_SEMICOLON);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_EQUAL");
    lua_pushinteger(L, GLFW_KEY_EQUAL);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_A");
    lua_pushinteger(L, GLFW_KEY_A);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_B");
    lua_pushinteger(L, GLFW_KEY_B);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_C");
    lua_pushinteger(L, GLFW_KEY_C);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_D");
    lua_pushinteger(L, GLFW_KEY_D);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_E");
    lua_pushinteger(L, GLFW_KEY_E);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F");
    lua_pushinteger(L, GLFW_KEY_F);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_G");
    lua_pushinteger(L, GLFW_KEY_G);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_H");
    lua_pushinteger(L, GLFW_KEY_H);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_I");
    lua_pushinteger(L, GLFW_KEY_I);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_J");
    lua_pushinteger(L, GLFW_KEY_J);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_K");
    lua_pushinteger(L, GLFW_KEY_K);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_L");
    lua_pushinteger(L, GLFW_KEY_L);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_M");
    lua_pushinteger(L, GLFW_KEY_M);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_N");
    lua_pushinteger(L, GLFW_KEY_N);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_O");
    lua_pushinteger(L, GLFW_KEY_O);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_P");
    lua_pushinteger(L, GLFW_KEY_P);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_Q");
    lua_pushinteger(L, GLFW_KEY_Q);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_R");
    lua_pushinteger(L, GLFW_KEY_R);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_S");
    lua_pushinteger(L, GLFW_KEY_S);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_T");
    lua_pushinteger(L, GLFW_KEY_T);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_U");
    lua_pushinteger(L, GLFW_KEY_U);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_V");
    lua_pushinteger(L, GLFW_KEY_V);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_W");
    lua_pushinteger(L, GLFW_KEY_W);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_X");
    lua_pushinteger(L, GLFW_KEY_X);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_Y");
    lua_pushinteger(L, GLFW_KEY_Y);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_Z");
    lua_pushinteger(L, GLFW_KEY_Z);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_LEFT_BRACKET");
    lua_pushinteger(L, GLFW_KEY_LEFT_BRACKET);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_BACKSLASH");
    lua_pushinteger(L, GLFW_KEY_BACKSLASH);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_RIGHT_BRACKET");
    lua_pushinteger(L, GLFW_KEY_RIGHT_BRACKET);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_GRAVE_ACCENT");
    lua_pushinteger(L, GLFW_KEY_GRAVE_ACCENT);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_WORLD_1");
    lua_pushinteger(L, GLFW_KEY_WORLD_1);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_WORLD_2");
    lua_pushinteger(L, GLFW_KEY_WORLD_2);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_ESCAPE");
    lua_pushinteger(L, GLFW_KEY_ESCAPE);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_ENTER");
    lua_pushinteger(L, GLFW_KEY_ENTER);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_TAB");
    lua_pushinteger(L, GLFW_KEY_TAB);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_BACKSPACE");
    lua_pushinteger(L, GLFW_KEY_BACKSPACE);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_INSERT");
    lua_pushinteger(L, GLFW_KEY_INSERT);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_DELETE");
    lua_pushinteger(L, GLFW_KEY_DELETE);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_RIGHT");
    lua_pushinteger(L, GLFW_KEY_RIGHT);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_LEFT");
    lua_pushinteger(L, GLFW_KEY_LEFT);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_DOWN");
    lua_pushinteger(L, GLFW_KEY_DOWN);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_UP");
    lua_pushinteger(L, GLFW_KEY_UP);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_PAGE_UP");
    lua_pushinteger(L, GLFW_KEY_PAGE_UP);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_PAGE_DOWN");
    lua_pushinteger(L, GLFW_KEY_PAGE_DOWN);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_HOME");
    lua_pushinteger(L, GLFW_KEY_HOME);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_END");
    lua_pushinteger(L, GLFW_KEY_END);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_CAPS_LOCK");
    lua_pushinteger(L, GLFW_KEY_CAPS_LOCK);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_SCROLL_LOCK");
    lua_pushinteger(L, GLFW_KEY_SCROLL_LOCK);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_NUM_LOCK");
    lua_pushinteger(L, GLFW_KEY_NUM_LOCK);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_PRINT_SCREEN");
    lua_pushinteger(L, GLFW_KEY_PRINT_SCREEN);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_PAUSE");
    lua_pushinteger(L, GLFW_KEY_PAUSE);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F1");
    lua_pushinteger(L, GLFW_KEY_F1);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F2");
    lua_pushinteger(L, GLFW_KEY_F2);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F3");
    lua_pushinteger(L, GLFW_KEY_F3);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F4");
    lua_pushinteger(L, GLFW_KEY_F4);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F5");
    lua_pushinteger(L, GLFW_KEY_F5);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F6");
    lua_pushinteger(L, GLFW_KEY_F6);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F7");
    lua_pushinteger(L, GLFW_KEY_F7);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F8");
    lua_pushinteger(L, GLFW_KEY_F8);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F9");
    lua_pushinteger(L, GLFW_KEY_F9);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F10");
    lua_pushinteger(L, GLFW_KEY_F10);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F11");
    lua_pushinteger(L, GLFW_KEY_F11);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F12");
    lua_pushinteger(L, GLFW_KEY_F12);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F13");
    lua_pushinteger(L, GLFW_KEY_F13);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F14");
    lua_pushinteger(L, GLFW_KEY_F14);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F15");
    lua_pushinteger(L, GLFW_KEY_F15);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F16");
    lua_pushinteger(L, GLFW_KEY_F16);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F17");
    lua_pushinteger(L, GLFW_KEY_F17);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F18");
    lua_pushinteger(L, GLFW_KEY_F18);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F19");
    lua_pushinteger(L, GLFW_KEY_F19);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F20");
    lua_pushinteger(L, GLFW_KEY_F20);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F21");
    lua_pushinteger(L, GLFW_KEY_F21);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F22");
    lua_pushinteger(L, GLFW_KEY_F22);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F23");
    lua_pushinteger(L, GLFW_KEY_F23);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F24");
    lua_pushinteger(L, GLFW_KEY_F24);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_F25");
    lua_pushinteger(L, GLFW_KEY_F25);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_0");
    lua_pushinteger(L, GLFW_KEY_KP_0);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_1");
    lua_pushinteger(L, GLFW_KEY_KP_1);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_2");
    lua_pushinteger(L, GLFW_KEY_KP_2);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_3");
    lua_pushinteger(L, GLFW_KEY_KP_3);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_4");
    lua_pushinteger(L, GLFW_KEY_KP_4);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_5");
    lua_pushinteger(L, GLFW_KEY_KP_5);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_6");
    lua_pushinteger(L, GLFW_KEY_KP_6);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_7");
    lua_pushinteger(L, GLFW_KEY_KP_7);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_8");
    lua_pushinteger(L, GLFW_KEY_KP_8);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_9");
    lua_pushinteger(L, GLFW_KEY_KP_9);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_DECIMAL");
    lua_pushinteger(L, GLFW_KEY_KP_DECIMAL);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_DIVIDE");
    lua_pushinteger(L, GLFW_KEY_KP_DIVIDE);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_MULTIPLY");
    lua_pushinteger(L, GLFW_KEY_KP_MULTIPLY);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_SUBTRACT");
    lua_pushinteger(L, GLFW_KEY_KP_SUBTRACT);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_ADD");
    lua_pushinteger(L, GLFW_KEY_KP_ADD);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_ENTER");
    lua_pushinteger(L, GLFW_KEY_KP_ENTER);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_KP_EQUAL");
    lua_pushinteger(L, GLFW_KEY_KP_EQUAL);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_LEFT_SHIFT");
    lua_pushinteger(L, GLFW_KEY_LEFT_SHIFT);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_LEFT_CONTROL");
    lua_pushinteger(L, GLFW_KEY_LEFT_CONTROL);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_LEFT_ALT");
    lua_pushinteger(L, GLFW_KEY_LEFT_ALT);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_LEFT_SUPER");
    lua_pushinteger(L, GLFW_KEY_LEFT_SUPER);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_RIGHT_SHIFT");
    lua_pushinteger(L, GLFW_KEY_RIGHT_SHIFT);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_RIGHT_CONTROL");
    lua_pushinteger(L, GLFW_KEY_RIGHT_CONTROL);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_RIGHT_ALT");
    lua_pushinteger(L, GLFW_KEY_RIGHT_ALT);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_RIGHT_SUPER");
    lua_pushinteger(L, GLFW_KEY_RIGHT_SUPER);
    lua_rawset(L, -3);

    lua_pushstring(L, "KEY_MENU");
    lua_pushinteger(L, GLFW_KEY_MENU);
    lua_rawset(L, -3);

    lua_rawset(L, -3);

    lua_setmetatable(L, -2);
    lua_setglobal(L, "window");
}
