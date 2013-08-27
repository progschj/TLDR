#ifndef PANEL_H
#define PANEL_H

#include <GL/glew.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <image.h>
#include <grid.h>

typedef struct panel_t {
    int width, height, font_width, font_height;
    GLubyte *background;
    GLubyte *foreground;
    GLuint fg_tex, bg_tex, font, vbo;
    GLhandleARB shader_program;
} panel;

panel panel_create(int width, int height, image font);

void panel_draw(panel *p, float x, float y, float width, float height);

void panel_set(panel *p, int x, int y, int tile, int fg, int bg);

void panel_set_tile(panel *p, int x, int y, int tile);

void panel_set_tile_fg(panel *p, int x, int y, int tile, int fg);

void panel_fill_rect(panel *p, int x0, int y0, int width, int height, int tile, int fg, int bg);

void panel_fill_rect_tile(panel *p, int x0, int y0, int width, int height, int tile);

void panel_fill_rect_tile_fg(panel *p, int x0, int y0, int width, int height, int tile, int fg);

void panel_fill_indexed(panel *p, grid *g, int (*tilespec)[3], int count);

void panel_fill_indexed_mul(panel *p, grid *g, grid *f, int (*tilespec)[3], int count);

void panel_xyprint(panel *p, int x0, int y0, const char *str);

void panel_destroy(panel p);

extern const luaL_Reg panel_lib[];

#endif
