#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <grid.h>

typedef double(*cost_function_t)(int,int,int,int,void*);
typedef struct pos_t {
    int x, y;
} pos;

double binary_grid_cost_function(int x0, int y0, int x1, int y1, void *userdata);

void dijkstra_distance(grid *distances, const pos *targets, int count, int neighbors, cost_function_t cost_function, void *userdata);

extern const luaL_Reg pathfinding_lib[];

#endif
