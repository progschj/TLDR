#include <grid.h>
#include <utility.h>

#include <math.h>

#define GRID_GET(G, A, B) grid_get((G), (data->swapped?data->signy*(B):data->signx*(A)), (data->swapped?data->signx*(A):data->signy*(B)))
#define GRID_SET(G, A, B, V) grid_set((G), (data->swapped?data->signy*(B):data->signx*(A)), (data->swapped?data->signx*(A):data->signy*(B)), (V))

static double dmin(double a, double b) {
    return a<b?a:b;
}

static double dmax(double a, double b) {
    return a>b?a:b;
}

typedef struct shadowcast_data_t {
    grid *transparent;
    grid *light;
    int r;
    double x0, y0;
    double signx, signy;
    int swapped;
    double falloff, amount;
    double (*attenuation)(struct shadowcast_data_t*,double,double);
} shadowcast_data;

static double flat_attenuation(struct shadowcast_data_t *data, double x, double y) {
    (void)data; (void)x; (void)y;
    return 1.0;
}

static double radial_attenuation(struct shadowcast_data_t *data, double x, double y) {
    double diffx = x+0.5 - data->x0;
    double diffy = y+0.5 - data->y0;
    double offset = 1.0/(1+data->falloff*data->r);
    return data->amount*dmax(0, (1.0/(1+data->falloff*sqrt(diffx*diffx + diffy*diffy))-offset)/(1-offset));
}

static void areasplit(double base, double slope, double *first, double *second) {
    if(slope < 1.0-base) {
         *first = 1.0-(base+0.5*slope);
         *second = 1.0;
    } else {
        double top = base+slope;
        double cut = (1.0-base)/slope;
        double w0 = 1.0-base;
        double h0 = cut;
        double w1 = top-1.0;
        double h1 = 1.0-cut;
        *first = 0.5f*w0*h0;
        *second = 1.0f-0.5f*w1*h1;
    }
}

static void fill_line(
    shadowcast_data *data, int height, double leftbase,
    double leftslope, double rightbase, double rightslope, double cond
)
{
    int ileft = floor(leftbase);
    int iright = floor(rightbase);

    double leftfirst, leftsecond, rightfirst, rightsecond;
    areasplit(leftbase-ileft, leftslope, &leftfirst, &leftsecond);
    areasplit(rightbase-iright, rightslope, &rightfirst, &rightsecond);

    if(GRID_GET(data->transparent, ileft, height)==cond)
        GRID_SET(data->light, ileft, height, GRID_GET(data->light, ileft, height) + leftfirst*data->attenuation(data, ileft, height));
    if(GRID_GET(data->transparent, ileft+1, height)==cond)
        GRID_SET(data->light, ileft+1, height, GRID_GET(data->light, ileft+1, height) + leftsecond*data->attenuation(data, ileft+1, height));
    if(GRID_GET(data->transparent, iright, height)==cond)
        GRID_SET(data->light, iright, height, GRID_GET(data->light, iright, height) - rightfirst*data->attenuation(data, iright, height));
    if(GRID_GET(data->transparent, iright+1, height)==cond)
        GRID_SET(data->light, iright+1, height, GRID_GET(data->light, iright+1, height) - rightsecond*data->attenuation(data, iright+1, height));

    for(int i = ileft+2;i<=iright+1;++i)
        if(GRID_GET(data->transparent, i, height)==cond)
            GRID_SET(data->light, i, height, GRID_GET(data->light, i, height) + data->attenuation(data, i, height));
}

static void set_line(
    shadowcast_data *data, int height, double leftbase,
    double leftslope, double rightbase, double rightslope, double cond
)
{
    int ileft = floor(leftbase);
    int iright = floor(rightbase);

    double leftfirst, leftsecond, rightfirst, rightsecond;
    areasplit(leftbase-ileft, leftslope, &leftfirst, &leftsecond);
    areasplit(rightbase-iright, rightslope, &rightfirst, &rightsecond);

    if(GRID_GET(data->transparent, ileft, height)==cond)
        GRID_SET(data->light, ileft, height, leftfirst*data->attenuation(data, ileft, height)>0?1.0:0.0);
    if(GRID_GET(data->transparent, ileft+1, height)==cond)
        GRID_SET(data->light, ileft+1, height, leftsecond*data->attenuation(data, ileft+1, height)>0?1.0:0.0);

    for(int i = ileft+2;i<=iright;++i)
        if(GRID_GET(data->transparent, i, height)==cond)
            GRID_SET(data->light, i, height, 1.0);
}

static void trace_cone(
    shadowcast_data *data, int height, int max_height, double leftbase,
    double leftslope, double rightbase, double rightslope
)
{
    (void)leftslope;
    if(height>max_height)
        return;
    int ileft = floor(leftbase);
    int iright = floor(rightbase+rightslope);

    double leftedge = leftbase;
    double rightedge;

    int i = ileft;
    
    set_line(data, height, leftbase, leftslope, rightbase, rightslope, 0.0);
    
    while(i<=iright) {
        while(i<=iright && !GRID_GET(data->transparent, i, height)) { leftedge = ++i; }
        rightedge = leftedge;
        while(i<=iright && GRID_GET(data->transparent, i, height)) {rightedge = ++i; }

        double newleftslope = (leftedge-data->x0)/(height-data->y0);
        double rightfillslope = dmin((rightedge-data->x0)/(height-data->y0), rightslope);
        double newrightslope = dmin((rightedge-data->x0)/(height+1-data->y0), rightslope);
        double newrightbase = dmin(rightedge, rightbase + rightslope);

        if(newleftslope < newrightslope)
        {
            fill_line(data, height, leftedge, newleftslope, dmin(rightedge, rightbase), rightfillslope, 1.0);
            trace_cone(data, height+1, max_height, leftedge+newleftslope, newleftslope, newrightbase, newrightslope);
        }
    }
}

static void calc_fov_octant(shadowcast_data *data) {
    double slope = 1;
    int start = floor(data->y0)+1;
    double fracy = start-data->y0;

    if(!GRID_GET(data->transparent, floor(data->x0)+1, floor(data->y0)))
        slope = dmin(slope, (1-(data->x0-floor(data->x0)))/(1-(data->y0-floor(data->y0))));
    trace_cone(data, start, start+data->r, data->x0, 0, data->x0+slope*fracy, slope);
}

void shadowcast_impl(
    grid *transparent, grid *light, double x0, double y0, int r,
    double falloff, double amount, double (*attenuation)(struct shadowcast_data_t*,double,double)
)
{
    //avoid issues at integer positions the cheap way
    if(x0 == floor(x0)) x0 = nextafter(x0, x0+1);
    if(y0 == floor(y0)) y0 = nextafter(y0, y0+1);

    double backup[] = {
        grid_get(light, x0, y0), grid_get(light, x0+1, y0),
        grid_get(light, x0-1, y0), grid_get(light, x0, y0+1),
        grid_get(light, x0, y0-1)
    };

    shadowcast_data data = {transparent, light, r, 0, 0, 0, 0, 0, falloff, amount, attenuation};

    data.x0 = 1-x0; data.y0 = y0; data.signx = -1; data.signy = 1; data.swapped = 0;
    calc_fov_octant(&data);

    data.x0 = x0; data.y0 = 1-y0; data.signx = 1; data.signy = -1; data.swapped = 0;
    calc_fov_octant(&data);

    data.x0 = 1-x0; data.y0 = 1-y0; data.signx = -1; data.signy = -1; data.swapped = 0;
    calc_fov_octant(&data);

    data.x0 = y0; data.y0 = x0; data.signx = 1; data.signy = 1; data.swapped = 1;
    calc_fov_octant(&data);

    data.x0 = 1-y0; data.y0 = x0; data.signx = -1; data.signy = 1; data.swapped = 1;
    calc_fov_octant(&data);

    data.x0 = y0; data.y0 = 1-x0; data.signx = 1; data.signy = -1; data.swapped = 1;
    calc_fov_octant(&data);

    data.x0 = 1-y0; data.y0 = 1-x0; data.signx = -1; data.signy = -1; data.swapped = 1;
    calc_fov_octant(&data);

    data.x0 = x0; data.y0 = y0; data.signx = 1; data.signy = 1; data.swapped = 0;
    calc_fov_octant(&data);

    
    grid_set(light, x0, y0, 1.0);
    if(grid_get(transparent, x0+1, y0))
        grid_set(light, x0+1, y0, backup[1]+data.attenuation(&data, floor(x0)+1, floor(y0)));
    if(grid_get(transparent, x0-1, y0))
        grid_set(light, x0-1, y0, backup[2]+data.attenuation(&data, floor(x0)-1, floor(y0)));
    if(grid_get(transparent, x0, y0+1))
        grid_set(light, x0, y0+1, backup[3]+data.attenuation(&data, floor(x0), floor(y0)+1));
    if(grid_get(transparent, x0, y0-1))
        grid_set(light, x0, y0-1, backup[4]+data.attenuation(&data, floor(x0), floor(y0)-1));
}

void shadowcast(grid *transparent, grid *light, double x0, double y0, int r)
{
    shadowcast_impl(transparent, light, x0, y0, r, 0, 1, flat_attenuation);
}

void attenuated_shadowcast(grid *transparent, grid *light, double x0, double y0, int r, double falloff, double amount)
{
    shadowcast_impl(transparent, light, x0, y0, r, falloff, amount, radial_attenuation);
}

int shadowcast_lua(lua_State *L) {
    check_userdata_type(L, 1, "grid");
    check_userdata_type(L, 2, "grid");
    int top = lua_gettop(L);
    if(top<5) typerror(L, top+1, "number");
    grid *transparent = lua_touserdata(L, 1);
    grid *light = lua_touserdata(L, 2);
    double x0 = lua_tonumber(L, 3);
    double y0 = lua_tonumber(L, 4);
    int r = lua_tointeger(L, 5);

    if(top == 5)
        shadowcast(transparent, light, x0, y0, r);
    else if(top == 6)
        attenuated_shadowcast(transparent, light, x0, y0, r, lua_tonumber(L, 6), 1);
    else
        attenuated_shadowcast(transparent, light, x0, y0, r, lua_tonumber(L, 6), lua_tonumber(L, 7));
    return 0;
}

double exact_raycast(grid *transparent, double x0, double y0, double x1, double y1) {
    double diffx = x1-x0;
    double diffy = y1-y0;

    double signx = diffx>0?1:-1;
    double signy = diffy>0?1:-1;
    double fx = diffx>0?1:0;
    double fy = diffy>0?1:0;
    
    double x = x0;
    double y = y0;
    int ix = floor(x);
    int iy = floor(y);
    while(grid_get(transparent, ix, iy) != 0.0 && signx*x<=signx*x1 && signy*y<=signy*y1) {
        double fractx = fx-(x-ix);
        double fracty = fy-(y-iy);
        double sx = fabs(diffx)>1.e-5 ? fractx/diffx : 1000;
        double sy = fabs(diffy)>1.e-5 ? fracty/diffy : 1000;
        if(sx < sy) {
            ix += signx;
        } else {
            iy += signy;
        }
        if(sx < sy) {
            x += fractx;
            y += fractx/diffx*diffy;
        } else {
            x += fracty/diffy*diffx;
            y += fracty;
        }
    }
    if(fabs(diffx)>fabs(diffy)) {
        return (x-x0)/(x1-x0);
    } else {
        return (y-y0)/(y1-y0);
    }
}

int raycast_lua(lua_State *L) {
    check_userdata_type(L, 1, "grid");
    int top = lua_gettop(L);
    if(top<5) typerror(L, top+1, "number");
    grid *transparent = lua_touserdata(L, 1);
    double x0 = lua_tonumber(L, 2);
    double y0 = lua_tonumber(L, 3);
    double x1 = lua_tonumber(L, 4);
    double y1 = lua_tonumber(L, 5);
    lua_pushnumber(L, exact_raycast(transparent, x0, y0, x1, y1));
    return 1;
}

const luaL_Reg shadowcast_lib[] = {
    {"shadowcast", shadowcast_lua},
    {"raycast", raycast_lua},
    {NULL, NULL}
};
