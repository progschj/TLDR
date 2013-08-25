#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <grid.h>
#include <utility.h>
#include <queue.h>

typedef double(*cost_function_t)(int,int,int,int,void*);
typedef struct pos_t {
    int x, y;
} pos;

const pos offsets[] = {
    { 1, 0}, {-1, 0}, { 0, 1}, { 0,-1},
    { 1, 1}, {-1, 1}, {-1,-1}, { 1,-1},
};

double binary_grid_cost_function(int x0, int y0, int x1, int y1, void *userdata) {
    grid *walkable = userdata;
    double diffx = x1-x0;
    double diffy = y1-y0;
    if(grid_get(walkable, x1, y1)) {
        return sqrt(diffx*diffx+diffy*diffy);
    } else {
        return DBL_MAX;
    }
}

void dijkstra_distance(grid *distances, const pos *targets, int count, int neighbors, cost_function_t cost_function, void *userdata){
    grid_fill(distances, distances->x0, distances->y0, distances->width, distances->height, DBL_MAX);

    queue open_set = queue_create(sizeof(pos));

    for(int i = 0;i<count;++i) {
        grid_set(distances, targets[i].x, targets[i].y, 0.0);
        queue_push(&open_set, targets+i);
    }

    while(!queue_empty(&open_set)) {
        pos cur = *(pos*)queue_front(&open_set);
        double curdist = grid_get(distances, cur.x, cur.y);
        queue_pop(&open_set);
        for(int i = 0;i<neighbors;++i) {
            pos neigh = {cur.x+offsets[i].x, cur.y+offsets[i].y};

            if(!grid_contains(distances, neigh.x, neigh.y)) continue;

            double neighdist = grid_get(distances, neigh.x, neigh.y);

            double cost = cost_function(cur.x, cur.y, neigh.x, neigh.y, userdata);
            if(neighdist > curdist + cost) {
                grid_set(distances, neigh.x, neigh.y, curdist + cost);
                queue_push(&open_set, &neigh);
            }
        }
    }

    queue_destroy(open_set);
}

static int dijkstra_lua(lua_State *L) {
    check_userdata_type(L, 1, "grid");
    check_userdata_type(L, 2, "grid");
    int top = lua_gettop(L);
    if(top<3 || !lua_istable(L, 3)) return typerror(L, 3, "table");
    grid *distances = lua_touserdata(L, 1);
    grid *walkable = lua_touserdata(L, 2);
    int count = lua_objlen(L, 3);
    pos *targets = malloc(count*sizeof(pos));
    for(int i = 1;i<=count;++i) {
        lua_pushinteger(L, i);
        lua_gettable(L, 3);

        lua_pushinteger(L, 1);
        lua_gettable(L, -2);
        targets[i-1].x = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_pushinteger(L, 2);
        lua_gettable(L, -2);
        targets[i-1].y = lua_tointeger(L, -1);
        lua_pop(L, 1);

    }
    int movement = 8;
    if(top>=4) movement = lua_tointeger(L, 4);
    dijkstra_distance(distances, targets, count, movement, binary_grid_cost_function, walkable);
    free(targets);
    return 0;
}

static int gradient_lua(lua_State *L) {
    int top = lua_gettop(L);
    check_userdata_type(L, 1, "grid");
    if(top<3) return typerror(L, 3, "number");
    grid *distances = lua_touserdata(L, 1);
    int x0 = lua_tonumber(L, 2);
    int y0 = lua_tonumber(L, 3);
    int movement = 8;
    if(top>=4) movement = lua_tointeger(L, 4);
    int minindex = -1;
    double minval = grid_get(distances, x0, y0);
    for(int i = 0;i<movement;++i) {
        int x = x0+offsets[i].x;
        int y = y0+offsets[i].y;
        if(!grid_contains(distances, x, y)) continue;
        double ival = grid_get(distances, x, y);
        if(ival<minval) {
            minval = ival;
            minindex = i;
        }
    }
    if(minindex == -1) {
        return 0;
    } else {
        lua_pushinteger(L, offsets[minindex].x);
        lua_pushinteger(L, offsets[minindex].y);
        return 2;
    }
}

const luaL_Reg pathfinding_lib[] = {
    {"dijkstra", dijkstra_lua},
    {"gradient", gradient_lua},

    {NULL, NULL}
};
