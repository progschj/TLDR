#include <image.h>

#include <stdlib.h>

#include <png.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <utility.h>


image image_load(const char *file_name) {
    image result = {0, 0, 0, NULL};
    png_byte header[8];

    FILE *fp = fopen(file_name, "rb");
    if(!fp) goto error;
    if(fread(header, 1, 8, fp) != 8 || png_sig_cmp(header, 0, 8)) goto error;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if(!png_ptr) goto error;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr) goto error;
    if(setjmp(png_jmpbuf(png_ptr))) goto error;
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING, NULL);

    result.width = png_get_image_width(png_ptr, info_ptr);
    result.height = png_get_image_height(png_ptr, info_ptr);
    result.components = png_get_channels(png_ptr,info_ptr);

    png_bytep *rows = png_get_rows(png_ptr, info_ptr);
    result.data = malloc(result.width*result.height*result.components);

    int i = 0;
    for(int h = 0;h<result.height;++h) {
        for(int w = 0;w<result.width*result.components;++w) {
            result.data[i++] = rows[h][w];
        }
    }
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    fclose(fp);

error:
    return result;
}

void image_destroy(image img) {
    free(img.data);
}

static int image_destroy_lua(lua_State *L) {
    image_destroy(*((image*)lua_touserdata(L, 1)));
    return 0;
}

static int image_size_lua(lua_State *L) {
    if(lua_gettop(L)<1) typerror(L, 1, "image");
    image *img = lua_touserdata(L, 1);
    lua_pushinteger(L, img->width);
    lua_pushinteger(L, img->height);
    return 2;
}

static int image_load_lua(lua_State *L) {
    if(lua_gettop(L)<1 || !lua_isstring(L, 1)) return typerror(L, 1, "string");
    image *img = lua_newuserdata(L, sizeof(image));
    *img = image_load(lua_tostring(L, 1));
    lua_newtable(L);
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, image_destroy_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_rawset(L, -3);

    lua_pushstring(L, "__type");
    lua_pushstring(L, "image");
    lua_rawset(L, -3);

    lua_pushstring(L, "size");
    lua_pushcfunction(L, image_size_lua);
    lua_rawset(L, -3);

    lua_setmetatable(L, -2);
    return 1;
}

const luaL_Reg image_lib[] = {
    {"load", image_load_lua},
    {NULL, NULL}
};
