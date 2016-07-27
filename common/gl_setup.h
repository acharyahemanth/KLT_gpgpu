#ifndef GLSETUPAPISH
#define GLSETUPAPISH

/*
Two libraries are used:
1) glfw : used to setup the GL window and get a GLcontext
2) gl3w : used to load gl function pointers and provide gl extensions
*/


#include "GL/gl3w.h"
#include <GLFW/glfw3.h>


GLFWwindow* setupGL(int window_width, int window_height, bool offscreen);
GLFWwindow* createContext(int window_width, int window_height, bool offscreen);
void loadGLExtensions();


#endif