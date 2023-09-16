#include <stdio.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

typedef char                b8;
typedef char                s8;
typedef short               s16;
typedef int                 s32;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;

typedef void gl_clear_fn(GLbitfield mask);
typedef void gl_clear_color_fn(GLclampf r, GLclampf g, GLclampf b, GLclampf a);

#define MEM_AMOUNT 1024*8

#define GAME_W  160
#define GAME_H  144
#define GAME_S  4
#define WINDOW_W (GAME_W * GAME_S)
#define WINDOW_H (GAME_H * GAME_S)
#define WINDOW_TITLE "GB11"

static u8 game_memory[MEM_AMOUNT]; /* basically all the memory we will ever need */

s32
main(void) {
  /* variables */
  GLFWwindow *window;
  const GLFWvidmode *vidmode;

  /* opengl declarations */
  gl_clear_fn *glClear;
  gl_clear_color_fn *glClearColor;

  (void)game_memory;

  /* init stuff */
  if (!glfwInit()) {
    const s8 *desc;
    glfwGetError(&desc);
    fprintf(stderr, "error: glfwInit: %s", desc);
    return -1;
  }

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if (!(window = glfwCreateWindow(WINDOW_W, WINDOW_H, WINDOW_TITLE, 0, 0))) {
    const s8 *desc;
    glfwGetError(&desc);
    fprintf(stderr, "error: glfwInit: %s", desc);
    return -1;
  }

  /* center window */
  vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  glfwSetWindowPos(window, (vidmode->width >> 1) - (WINDOW_W >> 1), (vidmode->height >> 1) - (WINDOW_H >> 1));
  glfwMakeContextCurrent(window);

  /* load opengl stuff */
  glClear      = (gl_clear_fn *)glfwGetProcAddress("glClear");
  glClearColor = (gl_clear_color_fn *)glfwGetProcAddress("glClearColor");

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.8f, 0.2, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  /* exit glfw */
  glfwTerminate();
  return 0;
}
