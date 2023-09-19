#include <stdarg.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <string.h>

/* typedefs */
typedef char                b8;
typedef char                s8;
typedef short               s16;
typedef int                 s32;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef float               f32;
typedef u32                 rgb;
typedef u8                  input;

#include "./atlas.h"

/* config */
#define GAME_W  160
#define GAME_H  144
#define GAME_TW (GAME_W >> 3)
#define GAME_TH (GAME_H >> 3)
#define GAME_S  4
#define WINDOW_W (GAME_W * GAME_S)
#define WINDOW_H (GAME_H * GAME_S)
#define WINDOW_TITLE "GB11"
#define TILE_SIZE  8
/* exit codes */
#define EXIT_GLFW     1
#define EXIT_WINDOW   2
#define EXIT_VERTEX   3
#define EXIT_FRAGMENT 4
#define EXIT_SHADER   5

/* structs */
typedef struct {
  u32 shader;
  b8 failed;
} shader_output;

#define BEGIN_TXT_LINES_CAP 5
typedef struct {
  u32 x, y;
  u32 w, h;
  s8 *begin_txt[BEGIN_TXT_LINES_CAP];
  s8 *map;
} level;

typedef struct {
  enum {
    FADE_NONE = 0,
    FADE_OUT,
    FADE_IN
  } fade_state;
  b8 is_happening;
} transition;

/* enums */
typedef enum {
  K_UP     = 1 << 0,
  K_LEFT   = 1 << 1,
  K_RIGHT  = 1 << 2,
  K_DOWN   = 1 << 3,
  K_A      = 1 << 4,
  K_B      = 1 << 5,
  K_START  = 1 << 6,
  K_SELECT = 1 << 7
} key;

typedef enum {
  WHITE = 0,
  LIGHT_GRAY,
  DARK_GRAY,
  BLACK,
  TRANSPARENT,
  COLORS_AMOUNT = TRANSPARENT
} color;

typedef enum {
  D_UP = 0,
  D_LEFT,
  D_RIGHT,
  D_DOWN
} direction;

/* opengl function types */
typedef void    gl_clear_fn(GLbitfield mask);
typedef void    gl_clear_color_fn(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
typedef GLuint  gl_create_shader_fn(GLenum type);
typedef void    gl_shader_source_fn(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void    gl_compile_shader_fn(GLuint shader);
typedef void    gl_get_shader_iv(GLuint shader, GLenum pname, GLint *params);
typedef void    gl_get_shader_info_log(GLuint shader, GLsizei max_length, GLsizei *length, GLchar *info_log);
typedef GLuint  gl_create_program_fn(void);
typedef void    gl_attach_shader_fn(GLuint program, GLuint shader);
typedef void    gl_link_program_fn(GLuint program);
typedef void    gl_get_program_iv_fn(GLuint program, GLenum pname, GLint *params);
typedef void    gl_get_program_info_log_fn(GLuint program, GLsizei max_length, GLsizei *length, GLchar *info_log);
typedef void    gl_use_program_fn(GLuint program);
typedef void    gl_gen_vertex_arrays_fn(GLsizei n, GLuint *arrays);
typedef void    gl_bind_vertex_array_fn(GLuint array);
typedef void    gl_gen_buffers_fn(GLsizei n, GLuint *buffers);
typedef void    gl_bind_buffer_fn(GLenum target, GLuint buffer);
typedef void    gl_buffer_data_fn(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void    gl_enable_vertex_attrib_array_fn(GLuint index);
typedef void    gl_vertex_attrib_pointer_fn(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void    gl_draw_elements_fn(GLenum mode, GLsizei count, GLenum type, const void *indices);
typedef void    gl_gen_textures_fn(GLsizei n, GLuint *textures);
typedef void    gl_bind_texture_fn(GLenum target, GLuint texture);
typedef void    gl_tex_parameteri_fn(GLenum target, GLenum pname, GLint param);
typedef void    gl_tex_image_2d(GLenum target, GLint level, GLint internal_format, GLsizei w, GLsizei h, GLint border, GLenum format, GLenum type, const void *data);
typedef void    gl_tex_sub_image_2d(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);

/* opengl functions */
gl_clear_fn                       *_glClear;
gl_clear_color_fn                 *_glClearColor;
gl_create_shader_fn               *_glCreateShader;
gl_shader_source_fn               *_glShaderSource;
gl_compile_shader_fn              *_glCompileShader;
gl_get_shader_iv                  *_glGetShaderiv;
gl_get_shader_info_log            *_glGetShaderInfoLog;
gl_create_program_fn              *_glCreateProgram;
gl_attach_shader_fn               *_glAttachShader;
gl_link_program_fn                *_glLinkProgram;
gl_get_program_iv_fn              *_glGetProgramiv;
gl_get_program_info_log_fn        *_glGetProgramInfoLog;
gl_use_program_fn                 *_glUseProgram;
gl_gen_vertex_arrays_fn           *_glGenVertexArrays;
gl_bind_vertex_array_fn           *_glBindVertexArray;
gl_gen_buffers_fn                 *_glGenBuffers;
gl_bind_buffer_fn                 *_glBindBuffer;
gl_buffer_data_fn                 *_glBufferData;
gl_enable_vertex_attrib_array_fn  *_glEnableVertexAttribArray;
gl_vertex_attrib_pointer_fn       *_glVertexAttribPointer;
gl_draw_elements_fn               *_glDrawElements;
gl_gen_textures_fn                *_glGenTextures;
gl_bind_texture_fn                *_glBindTexture;
gl_tex_parameteri_fn              *_glTexParameteri;
gl_tex_image_2d                   *_glTexImage2D;
gl_tex_sub_image_2d               *_glTexSubImage2D;

#define glClear                     _glClear
#define glClearColor                _glClearColor
#define glCreateShader              _glCreateShader
#define glShaderSource              _glShaderSource
#define glCompileShader             _glCompileShader
#define glGetShaderiv               _glGetShaderiv
#define glGetShaderInfoLog          _glGetShaderInfoLog
#define glCreateProgram             _glCreateProgram
#define glAttachShader              _glAttachShader
#define glLinkProgram               _glLinkProgram
#define glGetProgramiv              _glGetProgramiv
#define glGetProgramInfoLog         _glGetProgramInfoLog
#define glUseProgram                _glUseProgram
#define glGenVertexArrays           _glGenVertexArrays
#define glBindVertexArray           _glBindVertexArray
#define glGenBuffers                _glGenBuffers
#define glBindBuffer                _glBindBuffer
#define glBufferData                _glBufferData
#define glEnableVertexAttribArray   _glEnableVertexAttribArray
#define glVertexAttribPointer       _glVertexAttribPointer
#define glDrawElements              _glDrawElements
#define glGenTextures               _glGenTextures
#define glBindTexture               _glBindTexture
#define glTexParameteri             _glTexParameteri
#define glTexImage2D                _glTexImage2D
#define glTexSubImage2D             _glTexSubImage2D

/* input */
input key_cur;
input key_prv;
#define input_get(I, K) (((I) & (K)) == (K))
#define input_set(I, K) ((I) |=  (K))
#define input_clr(I, K) ((I) &= ~(K))
#define key_press(K) input_get(key_cur, K)
#define key_click(K) (input_get(key_cur, K) && !input_get(key_prv, K))

/* shader sources */
s8 *vert_src =
"#version 330 core\n"
"layout (location = 0) in vec2 a_pos;\n"
"layout (location = 1) in vec2 a_uv;\n"
"out vec2 v_uv;"
"void\n"
"main() {\n"
"  gl_Position = vec4(a_pos, 0.0, 1.0);\n"
"  v_uv = a_uv;"
"}\n"
"\n";
s8 *frag_src =
"#version 330 core\n"
"out vec4 f_col;\n"
"in vec2 v_uv;"
"uniform sampler2D tex;"
"void\n"
"main() {\n"
"  f_col = texture(tex, v_uv);\n"
"}\n"
"\n";

/* helper functions */
shader_output
make_shader(GLenum type, const s8 *src) {
  shader_output output;
  s32 shader_status;
  output.failed = 0;
  output.shader = glCreateShader(type);
  glShaderSource(output.shader, 1, &src, 0);
  glCompileShader(output.shader);
  glGetShaderiv(output.shader, GL_COMPILE_STATUS, &shader_status);
  if (!shader_status) {
    s8 info[1024];
    glGetShaderInfoLog(output.shader, 1024, 0, info);
    fprintf(stderr, "error: shader compilation: %s", info);
    output.failed = 1;
  }
  return output;
}

b8
rect_collide(s32 x1_min, s32 y1_min, s32 x1_max, s32 y1_max, s32 x2_min, s32 y2_min, s32 x2_max, s32 y2_max) {
  return x1_max > x2_min && x1_min < x2_max &&
         y1_max > y2_min && y1_min < y2_max;
}

/* callbacks */
void
key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods) {
  (void)window; (void)scancode; (void)mods;
  if (action == GLFW_REPEAT) return;
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_W: input_set(key_cur, K_UP);     break;
      case GLFW_KEY_A: input_set(key_cur, K_LEFT);   break;
      case GLFW_KEY_D: input_set(key_cur, K_RIGHT);  break;
      case GLFW_KEY_S: input_set(key_cur, K_DOWN);   break;
      case GLFW_KEY_J: input_set(key_cur, K_A);      break;
      case GLFW_KEY_K: input_set(key_cur, K_B);      break;
      case GLFW_KEY_U: input_set(key_cur, K_START);  break;
      case GLFW_KEY_I: input_set(key_cur, K_SELECT); break;
    }
  } else {
    switch (key) {
      case GLFW_KEY_W: input_clr(key_cur, K_UP);     break;
      case GLFW_KEY_A: input_clr(key_cur, K_LEFT);   break;
      case GLFW_KEY_D: input_clr(key_cur, K_RIGHT);  break;
      case GLFW_KEY_S: input_clr(key_cur, K_DOWN);   break;
      case GLFW_KEY_J: input_clr(key_cur, K_A);      break;
      case GLFW_KEY_K: input_clr(key_cur, K_B);      break;
      case GLFW_KEY_U: input_clr(key_cur, K_START);  break;
      case GLFW_KEY_I: input_clr(key_cur, K_SELECT); break;
    }
  }
}

/* renderer */
rgb screen[GAME_W*GAME_H];
rgb colors[COLORS_AMOUNT] = { 0x9bbc0f, 0x8bac0f, 0x306230, 0x0f380f };
rgb palette[COLORS_AMOUNT];
s32 bound_x_min, bound_y_min, bound_x_max, bound_y_max;

void
clear_screen(u8 color_index) {
  u32 i;
  for (i = 0; i < GAME_W*GAME_H; i++) {
    screen[i] = palette[color_index];
  }
}

void
set_drawing_bounds(s32 x_min, s32 y_min, s32 x_max, s32 y_max) {
  if (x_min < 0) x_min = 0;
  if (y_min < 0) y_min = 0;
  if (x_max > GAME_W) x_max = GAME_W;
  if (y_max > GAME_H) y_max = GAME_H;
  bound_x_min = x_min;
  bound_y_min = y_min;
  bound_x_max = x_max - 1;
  bound_y_max = y_max - 1;
}

void
reset_drawing_bounds(void) {
  bound_x_min = 0;
  bound_y_min = 0;
  bound_x_max = GAME_W - 1;
  bound_y_max = GAME_H - 1;
}

void
draw_rect(s32 x_min, s32 y_min, s32 x_max, s32 y_max, u8 color_index) {
  s32 x, y;
  if (x_max < bound_x_min || x_min > bound_x_max ||
      y_max < bound_y_min || y_min > bound_y_max) return;
  for (y = y_min; y < y_max; y++) {
    if (y < bound_y_min) continue;
    if (y > bound_y_max) break;
    for (x = x_min; x < x_max; x++) {
      if (x < bound_x_min) continue;
      if (x > bound_x_max) break;
      screen[y * GAME_W + x] = palette[color_index];
    }
  }
}

void
draw_tile(s32 x, s32 y, u32 tile_x, u32 tile_y) {
  u32 tx, ty;
  s32 px = x, py = y;
  if (x + TILE_SIZE < bound_x_min || x > bound_x_max ||
      y + TILE_SIZE < bound_y_min || y > bound_y_max) return;
  for (tx = 0; tx < ATLAS_H / TILE_SIZE; tx++) {
    for (ty = 0; ty < ATLAS_W / TILE_SIZE; ty++) {
      if (tx == tile_x && ty == tile_y) {
        u32 ox, oy;
        for (oy = 0; oy < TILE_SIZE; oy++) {
          py = y + oy;
          if (py < bound_y_min) continue;
          if (py > bound_y_max) break;
          for (ox = 0; ox < TILE_SIZE; ox++) {
            u8 color_index;
            s32 ax, ay;
            px = x + ox;
            if (px < bound_x_min) continue;
            if (px > bound_x_max) break;
            ax = (tx * TILE_SIZE + ox);
            ay = (ty * TILE_SIZE + oy);
            color_index = atlas[ay * ATLAS_W + ax];
            if (color_index < TRANSPARENT) {
              screen[py * GAME_W + px] = palette[color_index];
            }
          }
        }
      }
    }
  }
}

void
draw_text(s32 x, s32 y, s8 *fmt, ...) {
  u32 i;
  s8 txt[128];
  va_list args;
  va_start(args, fmt);
  vsprintf(txt, fmt, args);
  va_end(args);
  for (i = 0; i < 128; i++) {
    if (txt[i] == '\0') {
      break;
    } else if (txt[i] >= 'A' && txt[i] <= 'P') {
      draw_tile(x, y, txt[i] - 'A', 13);
    } else if (txt[i] >= 'Q' && txt[i] <= 'Z') {
      draw_tile(x, y, txt[i] - 'Q', 14);
    } else if (txt[i] >= '0' && txt[i] <= '1') {
      draw_tile(x, y, txt[i] - '0', 15);
    } else if (txt[i] != ' ') {
      switch (txt[i]) {
        case '.':  draw_tile(x, y, 10, 14); break;
        case ',':  draw_tile(x, y, 11, 14); break;
        case ':':  draw_tile(x, y, 12, 14); break;
        case ';':  draw_tile(x, y, 13, 14); break;
        case '?':  draw_tile(x, y, 14, 14); break;
        case '!':  draw_tile(x, y, 15, 14); break;
        case '-':  draw_tile(x, y, 10, 15); break;
        case '(':  draw_tile(x, y, 11, 15); break;
        case ')':  draw_tile(x, y, 12, 15); break;
        case '"':  draw_tile(x, y, 15, 15); break;
        case '\'': draw_tile(x, y, 14, 15); break;
        default:   draw_tile(x, y, 15, 15); break;
      }
    }
    x += TILE_SIZE;
  }
}

/* gameplay stuff */
f32 player_x,  player_y;
s32 player_nx, player_ny;
b8 player_walking;
direction player_dir;

s32 door_x, door_y;
s32 key_x, key_y;
b8 key_collected;

b8 grow_level;
direction grow_level_dir;
f32 level_x_min, level_x_max, level_y_min, level_y_max;
s32 level_nx_min, level_nx_max, level_ny_min, level_ny_max;

#define ARROWS_CAP 10
typedef struct {
  direction dir;
  s32 x, y;
  b8 collected;
} arrow;
arrow arrows[ARROWS_CAP];
u32 arrows_amount;

#define PLAYER_SPEED 80

#define LVL_MAX_W 20
#define LVL_MAX_H 18

#define TRANSITION_SPEED 0.2f

transition end_level;
transition begin_level;
f32 transition_timer;
u32 current_level;
level levels[] = {
  {
    0, 0,
    20, 18,
    {
      "WHEN EVERYTHING",
      "SEEMS ABOUT TO",
      "COLAPSE",
      0
    },
    "...................."
    "...................."
    "...................."
    ".............k......"
    "...................."
    "...................."
    "...................."
    "...................."
    "...................."
    ".........p.........."
    ".............v......"
    "...................."
    "...................."
    "....d..............."
    "...................."
    "...................."
    "...................."
    "...................."
  },
  {
    1, 1,
    18, 16,
    {
      "WHEN YOU LOOK",
      "AROUND AND ALL",
      "YOU SEE ARE CORNERS",
      0
    },
    ".................."
    ".................."
    ".................."
    ".....d............"
    ".................."
    ".................."
    "..............k..."
    ".................."
    ".................."
    ".........p........"
    ".................."
    ".................."
    ".................."
    ".................."
    ".................."
    ".................."
  },
  {
    0, 0,
    20, 18,
    {
      "THE FEELING OF",
      "COMPLETE",
      "CLAUSTROPHOBIA",
      "IS WHAT DRIVES",
      "YOU CRAZY"
    },
    "...................."
    "...................."
    "...................."
    "...................."
    "...................."
    "...........k........"
    "...................."
    "...................."
    "...................."
    ".........p.........."
    "...................."
    "...................."
    "....d..............."
    "...................."
    "...................."
    "...................."
    "...................."
    "...................."
  },
  {
    0, 0,
    20, 18,
    {
      "YOU NEED TO GET OUT",
      0
    },
    "...................."
    "...................."
    "...................."
    "...................."
    "...................."
    "...........k........"
    "...................."
    "...................."
    "...................."
    ".........p.........."
    "...................."
    "...................."
    "....d..............."
    "...................."
    "...................."
    "...................."
    "...................."
    "...................."
  },
  {
    0, 0,
    20, 18,
    {
      "OUT OF THE MADNESS",
      "OUT OF THIS PLANE",
      "OUT OF YOUR HEAD",
      0
    },
    "...................."
    "...................."
    "...................."
    "...................."
    "...................."
    "...........k........"
    "...................."
    "...................."
    "...................."
    ".........p.........."
    "...................."
    "...................."
    "....d..............."
    "...................."
    "...................."
    "...................."
    "...................."
    "...................."
  },
  {
    0, 0,
    20, 18,
    {
      "BUT YOU CAN'T...",
      0
    },
    "...................."
    "...................."
    "...................."
    "...................."
    "...................."
    "...........k........"
    "...................."
    "...................."
    "...................."
    ".........p.........."
    "...................."
    "...................."
    "....d..............."
    "...................."
    "...................."
    "...................."
    "...................."
    "...................."
  },
  {
    0, 0,
    20, 18,
    {
      "BUT I CAN'T...",
      0
    },
    "...................."
    "...................."
    "...................."
    "...................."
    "................k..."
    "...................."
    "...................."
    "...................."
    "...................."
    ".........p.........."
    "...................."
    "...................."
    "...................."
    "....d..............."
    "...................."
    "...................."
    "...................."
    "...................."
  },
};
u32 levels_begin_txt_lines_amount[sizeof (levels) / sizeof (level)];
s32 levels_begin_txt_x[sizeof (levels) / sizeof (level)][BEGIN_TXT_LINES_CAP];
s32 levels_begin_txt_y[sizeof (levels) / sizeof (level)][BEGIN_TXT_LINES_CAP];

void
add_arrow(direction dir, s32 x, s32 y) {
  if (arrows_amount == ARROWS_CAP) return;
  arrows[arrows_amount].x         = x;
  arrows[arrows_amount].y         = y;
  arrows[arrows_amount].dir       = dir;
  arrows[arrows_amount].collected = 0;
  arrows_amount++;
}

void
load_level(u32 level_idx) {
  u32 cx, cy;
  player_walking = 0;
  player_dir = 0;
  key_collected = 0;
  arrows_amount = 0;
  level_x_min = levels[level_idx].x * TILE_SIZE;
  level_y_min = levels[level_idx].y * TILE_SIZE;
  level_x_max = (levels[level_idx].x + levels[level_idx].w) * TILE_SIZE;
  level_y_max = (levels[level_idx].y + levels[level_idx].h) * TILE_SIZE;
  level_nx_min = level_x_min;
  level_ny_min = level_y_min;
  level_nx_max = level_x_max;
  level_ny_max = level_y_max;
  for (cy = 0; cy < levels[level_idx].w; cy++) {
    for (cx = 0; cx < levels[level_idx].h; cx++) {
      s32 x = (cx + levels[level_idx].x) * TILE_SIZE;
      s32 y = (cy + levels[level_idx].y) * TILE_SIZE ;
      switch (levels[level_idx].map[cy * levels[level_idx].w + cx]) {
        case 'p':
          player_x = x;
          player_y = y;
          player_nx = player_x;
          player_ny = player_y;
          break;
        case 'd':
          door_x = x;
          door_y = y;
          break;
        case 'k':
          key_x = x;
          key_y = y;
          break;
        case '^':
          add_arrow(D_UP, x, y);
          break;
        case '<':
          add_arrow(D_LEFT, x, y);
          break;
        case '>':
          add_arrow(D_RIGHT, x, y);
          break;
        case 'v':
          add_arrow(D_DOWN, x, y);
          break;
      }
    }
  }
  current_level = level_idx;
  reset_drawing_bounds();
}

void
init(void) {
  u32 i;
  end_level.is_happening = 1;
  end_level.fade_state = FADE_IN;
  begin_level.is_happening = 0;
  begin_level.fade_state = 0;
  transition_timer = 0;
  current_level = 0;
  grow_level = 0;
  for (i = 0; i < COLORS_AMOUNT; i++) palette[i] = colors[BLACK];
  for (i = 0; i < sizeof (levels) / sizeof (level); i++) {
    u32 j;
    u32 begin_txt_y;
    levels_begin_txt_lines_amount[i] = 0;
    for (j = 0; j < BEGIN_TXT_LINES_CAP; j++) {
      if (!levels[i].begin_txt[j]) break;
      levels_begin_txt_lines_amount[i]++;
    }
    begin_txt_y = (GAME_H >> 1) - ((levels_begin_txt_lines_amount[i] * TILE_SIZE) >> 1);
    for (j = 0; j < levels_begin_txt_lines_amount[i]; j++) {
      levels_begin_txt_x[i][j] = (GAME_W >> 1) - ((strlen(levels[i].begin_txt[j]) * TILE_SIZE) >> 1);
      levels_begin_txt_y[i][j] = begin_txt_y;
      begin_txt_y += TILE_SIZE;
    }
  }
  reset_drawing_bounds();
}

void
player_setup_movement(direction next_dir, s32 add_to_nx, s32 add_to_ny,
    b8 shrink_level_x_min, b8 shrink_level_y_min, b8 shrink_level_x_max, b8 shrink_level_y_max) {
  player_nx = player_x + add_to_nx;
  player_ny = player_y + add_to_ny;
  if (rect_collide(player_nx, player_ny, player_nx + TILE_SIZE, player_ny + TILE_SIZE, level_x_min, level_y_min, level_x_max, level_y_max) &&
    (key_collected || (player_nx != door_x || player_ny != door_y))) {
    player_dir = next_dir;
    player_walking = 1;
    level_nx_min = level_x_min + TILE_SIZE * shrink_level_x_min;
    level_ny_min = level_y_min + TILE_SIZE * shrink_level_y_min;
    level_nx_max = level_x_max - TILE_SIZE * shrink_level_x_max;
    level_ny_max = level_y_max - TILE_SIZE * shrink_level_y_max;
  }
}

void
player_move(b8 condition, f32 dt, s32 sign_x, s32 sign_y,
    b8 shrink_level_x_min, b8 shrink_level_y_min, b8 shrink_level_x_max, b8 shrink_level_y_max) {
  if (condition) {
    u32 i;
    player_y = player_ny;
    player_x = player_nx;
    player_walking = 0;
    level_x_min = level_nx_min;
    level_y_min = level_ny_min;
    level_x_max = level_nx_max;
    level_y_max = level_ny_max;
    if (key_collected) {
      if (player_x == door_x && player_y == door_y) {
        end_level.is_happening = 1;
        transition_timer = 0;
      }
    } else if (player_x == key_x && player_y == key_y) {
      key_collected = 1;
    }
    for (i = 0; i < arrows_amount; i++) {
      if (arrows[i].collected) continue;
      if (player_x == arrows[i].x && player_y == arrows[i].y) {
        arrows[i].collected = 1;
        grow_level_dir = arrows[i].dir;
        switch (arrows[i].dir) {
          case D_UP:
            level_ny_min = level_y_min - TILE_SIZE;
            grow_level = 1;
            break;
          case D_LEFT:
            level_nx_min = level_x_min - TILE_SIZE;
            grow_level = 1;
            break;
          case D_RIGHT:
            level_nx_max = level_x_max + TILE_SIZE;
            grow_level = 1;
            break;
          case D_DOWN:
            level_ny_max = level_y_max + TILE_SIZE;
            grow_level = 1;
            break;
        }
      }
    }
  } else {
    f32 delta_move = PLAYER_SPEED * dt;
    player_x += delta_move * sign_x;
    player_y += delta_move * sign_y;
    level_x_min += delta_move * shrink_level_x_min;
    level_y_min += delta_move * shrink_level_y_min;
    level_x_max -= delta_move * shrink_level_x_max;
    level_y_max -= delta_move * shrink_level_y_max;
  }
}

b8
fade_out(void) {
  if (palette[WHITE] == colors[WHITE]) {
    palette[WHITE]      = colors[LIGHT_GRAY];
    palette[LIGHT_GRAY] = colors[DARK_GRAY];
    palette[DARK_GRAY]  = colors[BLACK];
  } else if (palette[WHITE] == colors[LIGHT_GRAY]) {
    palette[WHITE]      = colors[DARK_GRAY];
    palette[LIGHT_GRAY] = colors[BLACK];
  } else if (palette[WHITE] == colors[DARK_GRAY]) {
    palette[WHITE]      = colors[BLACK];
  } else if (palette[WHITE] == colors[BLACK]) {
    return 1;
  }
  return 0;
}

b8
fade_in(void) {
  if (palette[WHITE] == colors[BLACK]) {
    palette[WHITE]      = colors[DARK_GRAY];
  } else if (palette[WHITE] == colors[DARK_GRAY]) {
    palette[WHITE]      = colors[LIGHT_GRAY];
    palette[LIGHT_GRAY] = colors[DARK_GRAY];
  } else if (palette[WHITE] == colors[LIGHT_GRAY]) {
    palette[WHITE]      = colors[WHITE];
    palette[LIGHT_GRAY] = colors[LIGHT_GRAY];
    palette[DARK_GRAY]  = colors[DARK_GRAY];
  } else if (palette[WHITE] == colors[WHITE]) {
    return 1;
  }
  return 0;
}

void
update(f32 dt) {
  /* level transitions */
  if (begin_level.is_happening) {
    if (begin_level.fade_state == FADE_NONE) {
      if (key_click(K_START)) begin_level.fade_state = FADE_OUT;
    } else {
      if (transition_timer < TRANSITION_SPEED) {
        transition_timer += dt;
      } else {
        transition_timer = 0;
        switch (begin_level.fade_state) {
          case FADE_OUT:
            if (fade_out()) {
              begin_level.fade_state = FADE_IN;
              load_level(current_level);
            }
            break;
          case FADE_IN:
            if (fade_in()) {
              begin_level.fade_state = FADE_NONE;
              begin_level.is_happening = 0;
            }
            break;
          default: break;
        }
      }
    }
    return;
  } else if (end_level.is_happening) {
    if (transition_timer < TRANSITION_SPEED) {
      transition_timer += dt;
    } else {
      transition_timer = 0;
      switch (end_level.fade_state) {
        case FADE_OUT:
          if (fade_out()) {
            end_level.fade_state = FADE_IN;
            current_level++;
          }
          break;
        case FADE_IN:
          if (fade_in()) {
            end_level.is_happening = 0;
            end_level.fade_state = FADE_OUT;
            begin_level.is_happening = 1;
          }
          break;
        default: break;
      }
    }
    return;
  }
  
  /* update level */
  if (grow_level) {
    switch (grow_level_dir) {
      case D_UP:
        if (level_y_min < level_ny_min) {
          level_y_min = level_ny_min;
          grow_level = 0;
        } else {
          level_y_min -= PLAYER_SPEED * dt;
        }
        break;
      case D_LEFT:
        if (level_x_min < level_nx_min) {
          level_x_min = level_nx_min;
          grow_level = 0;
        } else {
          level_x_min -= PLAYER_SPEED * dt;
        }
        break;
      case D_RIGHT:
        if (level_x_max > level_nx_max) {
          level_x_max = level_nx_max;
          grow_level = 0;
        } else {
          level_x_max += PLAYER_SPEED * dt;
        }
        break;
      case D_DOWN:
        if (level_y_max > level_ny_max) {
          level_y_max = level_ny_max;
          grow_level = 0;
        } else {
          level_y_max += PLAYER_SPEED * dt;
        }
        break;
    }
    return;
  }

  /* update player */
  if (!player_walking) {
    if (key_click(K_B))     load_level(current_level);
    if (key_click(K_UP))    player_setup_movement(D_UP,    0,        -TILE_SIZE, 0, 0, 0, 1);
    if (key_click(K_LEFT))  player_setup_movement(D_LEFT, -TILE_SIZE, 0        , 0, 0, 1, 0);
    if (key_click(K_DOWN))  player_setup_movement(D_DOWN,  0,         TILE_SIZE, 0, 1, 0, 0);
    if (key_click(K_RIGHT)) player_setup_movement(D_RIGHT, TILE_SIZE, 0        , 1, 0, 0, 0);
  } else {
    switch (player_dir) {
      case D_UP:    player_move(player_y < player_ny-1, dt,  0, -1, 0, 0, 0, 1); break;
      case D_LEFT:  player_move(player_x < player_nx-1, dt, -1,  0, 0, 0, 1, 0); break;
      case D_DOWN:  player_move(player_y > player_ny+1, dt,  0,  1, 0, 1, 0, 0); break;
      case D_RIGHT: player_move(player_x > player_nx+1, dt,  1,  0, 1, 0, 0, 0); break;
    }
  }
}

void
draw(void) {
  u32 i;
  if ((end_level.is_happening   && end_level.fade_state   != FADE_OUT) ||
      (begin_level.is_happening && begin_level.fade_state != FADE_IN)) {
    for (i = 0; i < levels_begin_txt_lines_amount[current_level]; i++) {
      draw_text(levels_begin_txt_x[current_level][i], levels_begin_txt_y[current_level][i], levels[current_level].begin_txt[i]);
    }
    return;
  }
  draw_rect(level_x_min, level_y_min, level_x_max, level_y_max, DARK_GRAY);
  set_drawing_bounds(level_x_min, level_y_min, level_x_max, level_y_max);
  draw_tile(door_x, door_y, key_collected, 2);
  if (!key_collected) draw_tile(key_x, key_y, 0, 1);
  for (i = 0; i < arrows_amount; i++) {
    if (!arrows[i].collected) draw_tile(arrows[i].x, arrows[i].y, arrows[i].dir, 3);
  }
  draw_tile(player_x, player_y, 0, 0);
  reset_drawing_bounds();
}


/* entry point */
s32
main(void) {
  /* variables */
  GLFWwindow *window;
  const GLFWvidmode *vidmode;
  u32 shader;
  u32 vao, vbo, ibo;
  u32 screen_tex;

  /* init stuff */
  if (!glfwInit()) {
    const s8 *desc;
    glfwGetError(&desc);
    fprintf(stderr, "error: glfwInit: %s", desc);
    return EXIT_GLFW;
  }

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if (!(window = glfwCreateWindow(WINDOW_W, WINDOW_H, WINDOW_TITLE, 0, 0))) {
    const s8 *desc;
    glfwGetError(&desc);
    fprintf(stderr, "error: glfwInit: %s", desc);
    return EXIT_WINDOW;
  }

  glfwSetKeyCallback(window, key_callback);

  /* center window */
  vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  glfwSetWindowPos(window, (vidmode->width >> 1) - (WINDOW_W >> 1), (vidmode->height >> 1) - (WINDOW_H >> 1));
  glfwMakeContextCurrent(window);

  /* load opengl stuff */
  glClear                    = (gl_clear_fn *)glfwGetProcAddress("glClear");
  glClearColor               = (gl_clear_color_fn *)glfwGetProcAddress("glClearColor");
  glCreateShader             = (gl_create_shader_fn *)glfwGetProcAddress("glCreateShader");
  glShaderSource             = (gl_shader_source_fn *)glfwGetProcAddress("glShaderSource");
  glCompileShader            = (gl_compile_shader_fn *)glfwGetProcAddress("glCompileShader");
  glGetShaderiv              = (gl_get_shader_iv *)glfwGetProcAddress("glGetShaderiv");
  glGetShaderInfoLog         = (gl_get_shader_info_log *)glfwGetProcAddress("glGetShaderInfoLog");
  glCreateProgram            = (gl_create_program_fn *)glfwGetProcAddress("glCreateProgram");
  glAttachShader             = (gl_attach_shader_fn *)glfwGetProcAddress("glAttachShader");
  glLinkProgram              = (gl_link_program_fn *)glfwGetProcAddress("glLinkProgram");
  glGetProgramiv             = (gl_get_program_iv_fn *)glfwGetProcAddress("glGetProgramiv");
  glGetProgramInfoLog        = (gl_get_program_info_log_fn *)glfwGetProcAddress("glGetProgramInfoLog");
  glUseProgram               = (gl_use_program_fn *)glfwGetProcAddress("glUseProgram");
  glGenVertexArrays          = (gl_gen_vertex_arrays_fn *)glfwGetProcAddress("glGenVertexArrays");
  glBindVertexArray          = (gl_bind_vertex_array_fn *)glfwGetProcAddress("glBindVertexArray");
  glGenBuffers               = (gl_gen_buffers_fn *)glfwGetProcAddress("glGenBuffers");
  glBindBuffer               = (gl_bind_buffer_fn *)glfwGetProcAddress("glBindBuffer");
  glBufferData               = (gl_buffer_data_fn *)glfwGetProcAddress("glBufferData");
  glEnableVertexAttribArray  = (gl_enable_vertex_attrib_array_fn *)glfwGetProcAddress("glEnableVertexAttribArray");
  glVertexAttribPointer      = (gl_vertex_attrib_pointer_fn *)glfwGetProcAddress("glVertexAttribPointer");
  glDrawElements             = (gl_draw_elements_fn *)glfwGetProcAddress("glDrawElements");
  glGenTextures              = (gl_gen_textures_fn *)glfwGetProcAddress("glGenTextures");
  glBindTexture              = (gl_bind_texture_fn *)glfwGetProcAddress("glBindTexture");
  glTexParameteri            = (gl_tex_parameteri_fn *)glfwGetProcAddress("glTexParameteri");
  glTexImage2D               = (gl_tex_image_2d *)glfwGetProcAddress("glTexImage2D");
  glTexSubImage2D            = (gl_tex_sub_image_2d *)glfwGetProcAddress("glTexSubImage2D");

  /* make shader */
  {
    shader_output vert;
    shader_output frag;
    s32 shader_status;
    vert = make_shader(GL_VERTEX_SHADER, vert_src);
    if (vert.failed) return EXIT_VERTEX;
    frag = make_shader(GL_FRAGMENT_SHADER, frag_src);
    if (frag.failed) return EXIT_FRAGMENT;
    shader = glCreateProgram();
    glAttachShader(shader, vert.shader);
    glAttachShader(shader, frag.shader);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &shader_status);
    if (!shader_status) {
      s8 info[1024];
      glGetProgramInfoLog(shader, 1024, 0, info);
      fprintf(stderr, "error: shader linking: %s", info);
      return EXIT_SHADER;
    }
    glUseProgram(shader);
  }

  /* buffers */
  {
    f32 vertices[] = {
    /* pos              uv    */
      -1.0f, -1.0f,     0, 1,
       1.0f, -1.0f,     1, 1,
       1.0f,  1.0f,     1, 0,
      -1.0f,  1.0f,     0, 0
    };
    u32 indices[] = {
      0, 1, 2,
      2, 3, 0
    };
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vao);
    glBufferData(GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof (f32) * 4, (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof (f32) * 4, (void *)(sizeof (f32) * 2));
  }

  /* screen */
  glGenTextures(1, &screen_tex);
  glBindTexture(GL_TEXTURE_2D, screen_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GAME_W, GAME_H, 0, GL_RGBA, GL_UNSIGNED_BYTE, screen);

  {
    f32 prv_time = glfwGetTime();
    init();
    while (!glfwWindowShouldClose(window)) {
      /* timing */
      f32 dt = glfwGetTime() - prv_time;
      prv_time = glfwGetTime();
      /* logic */
      update(dt);
      /* rendering */
      clear_screen(BLACK);
      draw();
      /* internal rendering/input */
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GAME_W, GAME_H, GL_RGBA, GL_UNSIGNED_BYTE, screen);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      glfwSwapBuffers(window);
      key_prv = key_cur;
      glfwPollEvents();
    }
  }

  /* exit glfw */
  glfwTerminate();
  return 0;
}
