#include <stdio.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

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

/* defines */
#define MEM_AMOUNT 1024*8
#define GAME_W  160
#define GAME_H  144
#define GAME_S  4
#define WINDOW_W (GAME_W * GAME_S)
#define WINDOW_H (GAME_H * GAME_S)
#define WINDOW_TITLE "GB11"
#define WHITE      0
#define LIGHT_GREY 1
#define DARK_GREY  2
#define BLACK      3

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

typedef struct {
  f32 x, y;
} v2;

enum {
  K_UP     = 1 << 0,
  K_LEFT   = 1 << 1,
  K_RIGHT  = 1 << 2,
  K_DOWN   = 1 << 3,
  K_A      = 1 << 4,
  K_B      = 1 << 5,
  K_START  = 1 << 6,
  K_SELECT = 1 << 7
};

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
static gl_clear_fn                       *_glClear;
static gl_clear_color_fn                 *_glClearColor;
static gl_create_shader_fn               *glCreateShader;
static gl_shader_source_fn               *glShaderSource;
static gl_compile_shader_fn              *glCompileShader;
static gl_get_shader_iv                  *glGetShaderiv;
static gl_get_shader_info_log            *glGetShaderInfoLog;
static gl_create_program_fn              *glCreateProgram;
static gl_attach_shader_fn               *glAttachShader;
static gl_link_program_fn                *glLinkProgram;
static gl_get_program_iv_fn              *glGetProgramiv;
static gl_get_program_info_log_fn        *glGetProgramInfoLog;
static gl_use_program_fn                 *glUseProgram;
static gl_gen_vertex_arrays_fn           *glGenVertexArrays;
static gl_bind_vertex_array_fn           *glBindVertexArray;
static gl_gen_buffers_fn                 *glGenBuffers;
static gl_bind_buffer_fn                 *glBindBuffer;
static gl_buffer_data_fn                 *glBufferData;
static gl_enable_vertex_attrib_array_fn  *glEnableVertexAttribArray;
static gl_vertex_attrib_pointer_fn       *glVertexAttribPointer;
static gl_draw_elements_fn               *_glDrawElements;
static gl_gen_textures_fn                *_glGenTextures;
static gl_bind_texture_fn                *_glBindTexture;
static gl_tex_parameteri_fn              *_glTexParameteri;
static gl_tex_image_2d                   *_glTexImage2D;
static gl_tex_sub_image_2d               *_glTexSubImage2D;
#define glClear          _glClear
#define glClearColor     _glClearColor 
#define glDrawElements   _glDrawElements
#define glGenTextures    _glGenTextures
#define glBindTexture    _glBindTexture
#define glTexParameteri  _glTexParameteri
#define glTexImage2D     _glTexImage2D
#define glTexSubImage2D  _glTexSubImage2D

/* global variables */
static u8 game_memory[MEM_AMOUNT]; /* basically all the memory we will ever need */
static rgb screen[GAME_W*GAME_H];
static rgb palette[4] = { 0x9bbc0f, 0x8bac0f, 0x306230, 0x0f380f };

/* input */
static input key_cur;
static input key_prv;
#define input_get(I, K) (((I) & (K)) == (K))
#define input_set(I, K) ((I) |=  (K))
#define input_clr(I, K) ((I) &= ~(K))
#define key_press(K) input_get(key_cur, K)
#define key_click(K) (input_get(key_cur, K) && !input_get(key_prv, K))

/* shader sources */
static s8 *vert_src =
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
static s8 *frag_src =
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
static shader_output
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

/* callbacks */
static void
key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods) {
  (void)window; (void)scancode; (void)mods;
  if (action == GLFW_REPEAT) return;
  if (action == GLFW_PRESS) {
    switch (key) {
      case GLFW_KEY_UP:    input_set(key_cur, K_UP);     break;
      case GLFW_KEY_LEFT:  input_set(key_cur, K_LEFT);   break;
      case GLFW_KEY_RIGHT: input_set(key_cur, K_RIGHT);  break;
      case GLFW_KEY_DOWN:  input_set(key_cur, K_DOWN);   break;
      case GLFW_KEY_Z:     input_set(key_cur, K_A);      break;
      case GLFW_KEY_X:     input_set(key_cur, K_B);      break;
      case GLFW_KEY_A:     input_set(key_cur, K_START);  break;
      case GLFW_KEY_S:     input_set(key_cur, K_SELECT); break;
    }
  } else {
    switch (key) {
      case GLFW_KEY_UP:    input_clr(key_cur, K_UP);     break;
      case GLFW_KEY_LEFT:  input_clr(key_cur, K_LEFT);   break;
      case GLFW_KEY_RIGHT: input_clr(key_cur, K_RIGHT);  break;
      case GLFW_KEY_DOWN:  input_clr(key_cur, K_DOWN);   break;
      case GLFW_KEY_Z:     input_clr(key_cur, K_A);      break;
      case GLFW_KEY_X:     input_clr(key_cur, K_B);      break;
      case GLFW_KEY_A:     input_clr(key_cur, K_START);  break;
      case GLFW_KEY_S:     input_clr(key_cur, K_SELECT); break;
    }
  }
}

/* renderer functions */
static void
clear_screen(u8 color_index) {
  u32 i;
  for (i = 0; i < GAME_W*GAME_H; i++) {
    screen[i] = palette[color_index];
  }
}

static void
draw_rect(v2 pos, u32 w, u32 h, u8 color_index) {
  s32 x_min, x_max;
  s32 y_min, y_max;
  s32 x, y;
  x_min = pos.x;
  x_max = x_min + w;
  y_min = pos.y;
  y_max = y_min + h;
  if (x_max < 0 || x_min > GAME_W - 1 ||
      y_max < 0 || y_min > GAME_H - 1) return;
  for (y = y_min; y < y_max; y++) {
    if (y < 0) continue;
    if (y > GAME_H) break;
    for (x = x_min; x < x_max; x++) {
      if (x < 0) continue;
      if (x > GAME_W) break;
      screen[y * GAME_W + x] = palette[color_index];
    }
  }
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

  (void)game_memory;

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
    /* game variables */
    v2 player_pos = { 6, 5 };
    v2 level_pos = { 2, 2 };
    u32 level_w = GAME_W - 4, level_h = GAME_H - 4;
    f32 dt = 0;
    f32 prv_time = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
      /* timing */
      dt = glfwGetTime() - prv_time;
      prv_time = glfwGetTime();
      /* logic */
      if (key_press(K_RIGHT)) player_pos.x += 50 * dt;
      if (key_press(K_LEFT))  player_pos.x -= 50 * dt;
      if (key_press(K_UP))    player_pos.y -= 50 * dt;
      if (key_press(K_DOWN))  player_pos.y += 50 * dt;
      /* rendering */
      clear_screen(BLACK);
      draw_rect(level_pos, level_w, level_h, DARK_GREY);
      draw_rect(player_pos, 10, 10, LIGHT_GREY);
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
