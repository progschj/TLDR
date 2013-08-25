#include <panel.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <GL/glew.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <utility.h>
#include <image.h>
#include <grid.h>

static GLuint texture_create(image img) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    switch(img.components) {
        case 1:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, img.width, img.height, 0, GL_RED, GL_UNSIGNED_BYTE, img.data);
            break;
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
            break;
    }

    return tex;
}

const char *vertex_source =
    "#version 110\n"
    "uniform vec4 extents;\n"
    "void main() {\n"
    "   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "   gl_Position = vec4(2.0*(extents.xy + gl_Vertex.xy*extents.zw)-1.0, 0, 1);\n"
    "}\n";

const char *fragment_source =
    "#version 110\n"
    "uniform sampler2D font;\n"
    "uniform sampler2D foreground;\n"
    "uniform sampler2D background;\n"
    "uniform vec2 panel_size;\n"

    "void main() {\n"
    "   vec2 tilecoord = fract(gl_TexCoord[0].xy*panel_size);\n"
    "   vec4 fg = texture2D(foreground, gl_TexCoord[0].xy);\n"
    "   vec4 bg = texture2D(background, gl_TexCoord[0].xy);\n"
    "   vec2 tile = vec2(fg.w, bg.w)*255./256.;\n"
    "   vec4 glyph = texture2D(font, tile.xy+vec2(1./16.)*tilecoord);\n"
    "   gl_FragColor.xyz = mix(bg.xyz, fg.xyz, glyph.xyz);\n"
    "   gl_FragColor.w = 1.;\n"
    "}\n";

// data for a fullscreen quad
const GLfloat quad_data[] = {
//  X     Y     Z           U     V
   1.0f, 1.0f, 0.0f,       1.0f, 0.0f,
   0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
   1.0f, 0.0f, 0.0f,       1.0f, 1.0f,
   1.0f, 0.0f, 0.0f,       1.0f, 1.0f,
   0.0f, 1.0f, 0.0f,       0.0f, 0.0f,
   0.0f, 0.0f, 0.0f,       0.0f, 1.0f,
}; // 6 vertices with 6 components (floats) each

panel panel_create(int width, int height, image font) {
    panel result = {width, height, font.width, font.height, NULL, NULL, 0, 0, 0, 0, 0};

    result.font = texture_create(font);
    result.foreground = malloc(4*width*height);
    result.background = malloc(4*width*height);

    // program and shader handles
    GLhandleARB vertex_shader, fragment_shader;

    int length;
    GLint shader_ok;

    // create and compiler vertex shader
    vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    length = strlen(vertex_source);
    glShaderSourceARB(vertex_shader, 1, &vertex_source, &length);
    glCompileShaderARB(vertex_shader);
    glGetObjectParameterivARB(vertex_shader, GL_COMPILE_STATUS, &shader_ok);
    assert(shader_ok);

    // create and compiler fragment shader
    fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    length = strlen(fragment_source);
    glShaderSourceARB(fragment_shader, 1, &fragment_source, &length);
    glCompileShaderARB(fragment_shader);
    glGetObjectParameterivARB(fragment_shader, GL_COMPILE_STATUS, &shader_ok);
    assert(shader_ok);

    // create program
    result.shader_program = glCreateProgramObjectARB();

    // attach shaders
    glAttachObjectARB(result.shader_program, vertex_shader);
    glAttachObjectARB(result.shader_program, fragment_shader);

    // link the program and check for errors
    glLinkProgramARB(result.shader_program);
    glGetObjectParameterivARB(result.shader_program, GL_LINK_STATUS, &shader_ok);
    assert(shader_ok);

    // we don't need these anymore
    glDeleteObjectARB(vertex_shader);
    glDeleteObjectARB(fragment_shader);

    // generate and bind the buffer object
    glGenBuffersARB(1, &result.vbo);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, result.vbo);

    // fill with data
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(GLfloat)*6*5, quad_data, GL_STATIC_DRAW_ARB);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &result.fg_tex);
    glBindTexture(GL_TEXTURE_2D, result.fg_tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, result.width, result.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, result.foreground);

    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &result.bg_tex);
    glBindTexture(GL_TEXTURE_2D, result.bg_tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, result.width, result.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, result.background);

    glUseProgramObjectARB(result.shader_program);

    glUniform1iARB(glGetUniformLocationARB(result.shader_program, "font"), 0);
    glUniform1iARB(glGetUniformLocationARB(result.shader_program, "foreground"), 1);
    glUniform1iARB(glGetUniformLocationARB(result.shader_program, "background"), 2);
    glUniform2fARB(glGetUniformLocationARB(result.shader_program, "panel_size"), width, height);

    return result;
}

void panel_draw(panel *p, float x, float y, float width, float height) {
    glUseProgramObjectARB(p->shader_program);
    glUniform4fARB(glGetUniformLocationARB(p->shader_program, "extents"), x, 1.0f-y-height, width, height);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, p->font);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, p->fg_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, p->width, p->height, GL_RGBA, GL_UNSIGNED_BYTE, p->foreground);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, p->bg_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, p->width, p->height, GL_RGBA, GL_UNSIGNED_BYTE, p->background);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, p->vbo);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 5*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 5*sizeof(GLfloat), (char*)0 + 3*sizeof(GLfloat));

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void panel_set(panel *p, int x, int y, int tile, int fg, int bg) {
    if(x<0 || x>=p->width || y<0 || y>=p->height) return;
    int index = x + p->width*y;

    p->foreground[4*index + 0] = (fg>>16) & 0xFF;
    p->foreground[4*index + 1] = (fg>> 8) & 0xFF;
    p->foreground[4*index + 2] = (fg>> 0) & 0xFF;
    p->foreground[4*index + 3] = 16*((tile&0xFF)%16);

    p->background[4*index + 0] = (bg>>16) & 0xFF;
    p->background[4*index + 1] = (bg>> 8) & 0xFF;
    p->background[4*index + 2] = (bg>> 0) & 0xFF;
    p->background[4*index + 3] = 16*((tile&0xFF)/16);
}

void panel_set_tile(panel *p, int x, int y, int tile) {
    if(x<0 || x>=p->width || y<0 || y>=p->height) return;
    int index = x + p->width*y;

    p->foreground[4*index + 3] = 16*((tile&0xFF)%16);
    p->background[4*index + 3] = 16*((tile&0xFF)/16);
}

void panel_set_tile_fg(panel *p, int x, int y, int tile, int fg) {
    if(x<0 || x>=p->width || y<0 || y>=p->height) return;
    int index = x + p->width*y;

    p->foreground[4*index + 0] = (fg>>16) & 0xFF;
    p->foreground[4*index + 1] = (fg>> 8) & 0xFF;
    p->foreground[4*index + 2] = (fg>> 0) & 0xFF;
    p->foreground[4*index + 3] = 16*((tile&0xFF)%16);
    p->background[4*index + 3] = 16*((tile&0xFF)/16);
}

void panel_fill_rect(panel *p, int x0, int y0, int width, int height, int tile, int fg, int bg) {
    x0 = clamp(x0, 0, p->width);
    int x1 = clamp(x0+width, 0, p->width);
    y0 = clamp(y0, 0, p->height);
    int y1 = clamp(y0+height, 0, p->height);
    for(int x = x0;x<x1;++x) {
        for(int y = y0;y<y1;++y) {
            int index = x + p->width*y;

            p->foreground[4*index + 0] = (fg>>16) & 0xFF;
            p->foreground[4*index + 1] = (fg>> 8) & 0xFF;
            p->foreground[4*index + 2] = (fg>> 0) & 0xFF;
            p->foreground[4*index + 3] = 16*((tile&0xFF)%16);

            p->background[4*index + 0] = (bg>>16) & 0xFF;
            p->background[4*index + 1] = (bg>> 8) & 0xFF;
            p->background[4*index + 2] = (bg>> 0) & 0xFF;
            p->background[4*index + 3] = 16*((tile&0xFF)/16);
        }
    }
}

void panel_fill_rect_tile(panel *p, int x0, int y0, int width, int height, int tile) {
    x0 = clamp(x0, 0, p->width);
    int x1 = clamp(x0+width, 0, p->width);
    y0 = clamp(y0, 0, p->height);
    int y1 = clamp(y0+height, 0, p->height);
    for(int x = x0;x<x1;++x) {
        for(int y = y0;y<y1;++y) {
            int index = x + p->width*y;
            p->foreground[4*index + 3] = 16*((tile&0xFF)%16);
            p->background[4*index + 3] = 16*((tile&0xFF)/16);
        }
    }
}

void panel_fill_rect_tile_fg(panel *p, int x0, int y0, int width, int height, int tile, int fg) {
    x0 = clamp(x0, 0, p->width);
    int x1 = clamp(x0+width, 0, p->width);
    y0 = clamp(y0, 0, p->height);
    int y1 = clamp(y0+height, 0, p->height);
    for(int x = x0;x<x1;++x) {
        for(int y = y0;y<y1;++y) {
            int index = x + p->width*y;

            p->foreground[4*index + 0] = (fg>>16) & 0xFF;
            p->foreground[4*index + 1] = (fg>> 8) & 0xFF;
            p->foreground[4*index + 2] = (fg>> 0) & 0xFF;
            p->foreground[4*index + 3] = 16*((tile&0xFF)%16);
            p->background[4*index + 3] = 16*((tile&0xFF)/16);
        }
    }
}

void panel_fill_indexed(panel *p, grid *g, int (*tilespec)[3]) {
    int x0 = clamp(g->x0, 0, p->width);
    int x1 = clamp(g->x0+g->width, 0, p->width);
    int y0 = clamp(g->y0, 0, p->height);
    int y1 = clamp(g->y0+g->height, 0, p->height);
    for(int x = x0;x<x1;++x) {
        for(int y = y0;y<y1;++y) {
            int index = x + p->width*y;
            int k = grid_get(g, x, y);
            int tile = tilespec[k][0];
            int fg = tilespec[k][1];
            int bg = tilespec[k][2];
            if(tile >= 0) {
                p->foreground[4*index + 3] = 16*((tile&0xFF)%16);
                p->background[4*index + 3] = 16*((tile&0xFF)/16);
            }
            if(fg >= 0) {
                p->foreground[4*index + 0] = (fg>>16) & 0xFF;
                p->foreground[4*index + 1] = (fg>> 8) & 0xFF;
                p->foreground[4*index + 2] = (fg>> 0) & 0xFF;
            }
            if(bg >= 0) {
                p->background[4*index + 0] = (bg>>16) & 0xFF;
                p->background[4*index + 1] = (bg>> 8) & 0xFF;
                p->background[4*index + 2] = (bg>> 0) & 0xFF;
            }
        }
    }
}

void panel_fill_indexed_mul(panel *p, grid *g, grid *f, int (*tilespec)[3]) {
    int x0 = clamp(g->x0, 0, p->width);
    int x1 = clamp(g->x0+g->width, 0, p->width);
    int y0 = clamp(g->y0, 0, p->height);
    int y1 = clamp(g->y0+g->height, 0, p->height);
    for(int x = x0;x<x1;++x) {
        for(int y = y0;y<y1;++y) {
            int index = x + p->width*y;
            int k = grid_get(g, x, y);
            double factor = grid_get(f, x, y);
            int tile = tilespec[k][0];
            int fg = tilespec[k][1];
            int bg = tilespec[k][2];
            if(tile >= 0) {
                p->foreground[4*index + 3] = 16*((tile&0xFF)%16);
                p->background[4*index + 3] = 16*((tile&0xFF)/16);
            }
            if(fg >= 0) {
                p->foreground[4*index + 0] = clamp(factor*((fg>>16) & 0xFF), 0, 0xFF);
                p->foreground[4*index + 1] = clamp(factor*((fg>> 8) & 0xFF), 0, 0xFF);
                p->foreground[4*index + 2] = clamp(factor*((fg>> 0) & 0xFF), 0, 0xFF);
            }
            if(bg >= 0) {
                p->background[4*index + 0] = clamp(factor*((bg>>16) & 0xFF), 0, 0xFF);
                p->background[4*index + 1] = clamp(factor*((bg>> 8) & 0xFF), 0, 0xFF);
                p->background[4*index + 2] = clamp(factor*((bg>> 0) & 0xFF), 0, 0xFF);
            }
        }
    }    
}


void panel_print(panel *p, int x0, int y0, const char *str) {
    int fg = -1, bg = -1;
    for(int x = x0, y = y0;*str != '\0';++str) {
        if(*str == '\n') {
            x = x0; ++y; continue;
        } else if(*str == '\\') {
            ++str;
            char *endptr;
            switch(*str) {
                case '\\': break;
                case '\0': return;
                case 'f':
                    fg = strtol(str+1, &endptr, 16);
                    if(endptr == str+2) fg = -1;
                    str = endptr-1;
                    continue;
                case 'b':
                    bg = strtol(str+1, &endptr, 16);
                    if(endptr == str+2) bg = -1;
                    str = endptr-1;
                    continue;
            }
        }
        if(x<0 || x>=p->width || y<0 || y>=p->height) {
            continue;
        }
        int index = x + p->width*y;

        p->foreground[4*index + 3] = 16*((*str&0xFF)%16);
        p->background[4*index + 3] = 16*((*str&0xFF)/16);

        if(fg>0) {
            p->foreground[4*index + 0] = (fg>>16) & 0xFF;
            p->foreground[4*index + 1] = (fg>> 8) & 0xFF;
            p->foreground[4*index + 2] = (fg>> 0) & 0xFF;
        }

        if(bg>0) {
            p->background[4*index + 0] = (bg>>16) & 0xFF;
            p->background[4*index + 1] = (bg>> 8) & 0xFF;
            p->background[4*index + 2] = (bg>> 0) & 0xFF;
        }
        ++x;
    }
}

void panel_destroy(panel p) {
    free(p.foreground);
    free(p.background);
    glDeleteObjectARB(p.shader_program);
    glDeleteTextures(1, &p.fg_tex);
    glDeleteTextures(1, &p.bg_tex);
    glDeleteTextures(1, &p.font);
    glDeleteBuffersARB(1, &p.vbo);
}


static int panel_destroy_lua(lua_State *L) {
    panel_destroy(*((panel*)lua_touserdata(L, 1)));
    return 0;
}

static int panel_size_lua(lua_State *L) {
    if(lua_gettop(L)<1) typerror(L, 1, "panel");
    panel *p = lua_touserdata(L, 1);
    lua_pushinteger(L, p->width);
    lua_pushinteger(L, p->height);
    return 2;
}

static int panel_pixelsize_lua(lua_State *L) {
    if(lua_gettop(L)<1) typerror(L, 1, "panel");
    panel *p = lua_touserdata(L, 1);
    lua_pushinteger(L, p->width*p->font_width/16);
    lua_pushinteger(L, p->height*p->font_height/16);
    return 2;
}

static int panel_set_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "panel");
    panel *p = lua_touserdata(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    int tile = lua_tointeger(L, 4);
    int fg = lua_tointeger(L, 5);
    int bg = lua_tointeger(L, 6);
    if(top<4) {
        return 0;
    } else if(top == 4) {
        panel_set_tile(p, x, y, tile);
    } else if(top == 5) {
        panel_set_tile_fg(p, x, y, tile, fg);
    } else {
        panel_set(p, x, y, tile, fg, bg);
    }
    return 0;
}

static int panel_fillrect_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "panel");
    panel *p = lua_touserdata(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    int width = lua_tointeger(L, 4);
    int height = lua_tointeger(L, 5);
    int tile = lua_tointeger(L, 6);
    int fg = lua_tointeger(L, 7);
    int bg = lua_tointeger(L, 8);
    if(top<6) {
        return 0;
    } else if(top == 6) {
        panel_fill_rect_tile(p, x, y, width, height, tile);
    } else if(top == 7) {
        panel_fill_rect_tile_fg(p, x, y, width, height, tile, fg);
    } else {
        panel_fill_rect(p, x, y, width, height, tile, fg, bg);
    }
    return 0;
}

static int panel_fill_indexed_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "panel");
    check_userdata_type(L, 2, "grid");
    if(!lua_istable(L, top)) typerror(L, top, "table");
    panel *p = lua_touserdata(L, 1);
    grid *g = lua_touserdata(L, 2);
    int count = lua_objlen(L, top);
    int (*tilespec)[3] = malloc(sizeof(int[3])*count);
    for(int i = 1;i<=count;++i) {
        lua_pushinteger(L, i);
        lua_gettable(L, top);

        lua_pushinteger(L, 1);
        lua_gettable(L, -2);
        tilespec[i-1][0] = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_pushinteger(L, 2);
        lua_gettable(L, -2);
        tilespec[i-1][1] = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_pushinteger(L, 3);
        lua_gettable(L, -2);
        tilespec[i-1][2] = lua_tointeger(L, -1);
        lua_pop(L, 1);
    }
    if(top>3) {
        check_userdata_type(L, 3, "grid");
        panel_fill_indexed_mul(p, g, lua_touserdata(L, 3), tilespec);
    } else {
        panel_fill_indexed(p, g, tilespec);
    }
    free(tilespec);
    return 0;
}

static int panel_print_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "panel");
    if(top<3) typerror(L, top+1, "number");
    if(top<4) typerror(L, top+1, "string");
    panel *p = lua_touserdata(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    const char *str = lua_tostring(L, 4);
    panel_print(p, x, y, str);
    return 0;
}

static int panel_draw_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1) typerror(L, 1, "panel");
    panel *p = lua_touserdata(L, 1);
    double x = top<2 ? 0.0f : lua_tonumber(L, 2);
    double y = top<3 ? 0.0f : lua_tonumber(L, 3);
    double width = top<4 ? 1.0f : lua_tonumber(L, 4);
    double height = top<5 ? 1.0f : lua_tonumber(L, 5);
    panel_draw(p, x, y, width, height);
    return 0;
}

static int panel_create_lua(lua_State *L) {
    int top = lua_gettop(L);
    if(top<1 || !lua_isnumber(L, 1)) return typerror(L, 1, "number");
    if(top<2 || !lua_isnumber(L, 2)) return typerror(L, 2, "number");
    check_userdata_type(L, 3, "image");
    panel *p = lua_newuserdata(L, sizeof(panel));
    *p = panel_create(lua_tointeger(L, 1), lua_tointeger(L, 2), *((image*)lua_touserdata(L, 3)));
    lua_newtable(L);
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, panel_destroy_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);
    lua_rawset(L, -3);

    lua_pushstring(L, "__type");
    lua_pushstring(L, "panel");
    lua_rawset(L, -3);

    lua_pushstring(L, "size");
    lua_pushcfunction(L, panel_size_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "pixel_size");
    lua_pushcfunction(L, panel_pixelsize_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "set");
    lua_pushcfunction(L, panel_set_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "fill_rect");
    lua_pushcfunction(L, panel_fillrect_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "fill_indexed");
    lua_pushcfunction(L, panel_fill_indexed_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "print");
    lua_pushcfunction(L, panel_print_lua);
    lua_rawset(L, -3);

    lua_pushstring(L, "draw");
    lua_pushcfunction(L, panel_draw_lua);
    lua_rawset(L, -3);

    lua_setmetatable(L, -2);
    return 1;
}

const luaL_Reg panel_lib[] = {
    {"create", panel_create_lua},
    {NULL, NULL}
};
