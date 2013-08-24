#ifndef GRID_H
#define GRID_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct grid_t {
    int x0, y0, width, height;
    double *data;
} grid;

grid grid_create(int x0, int y0, int width, int height);

grid grid_clone(grid *g0);

int grid_contains(grid *g, int x, int y);

double grid_get(grid *g, int x, int y);

void grid_set(grid *g, int x, int y, double v);

void grid_fill(grid *g, int x, int y, int width, int height, double v);

void grid_jitter(grid *g, double amount);

void grid_destroy(grid g);

extern const luaL_Reg grid_lib[];

#endif
