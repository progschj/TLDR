#ifndef IMAGE_H
#define IMAGE_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct image_t {
    int width, height, components;
    char *data;
} image;

image image_load(const char *file_name);

void image_destroy(image img);

extern const luaL_Reg image_lib[];

#endif
