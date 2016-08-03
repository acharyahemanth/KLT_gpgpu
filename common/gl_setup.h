#ifndef GLSETUPAPISH
#define GLSETUPAPISH

/*
Two libraries are used:
1) glfw : used to setup the GL window and get a GLcontext
2) gl3w : used to load gl function pointers and provide gl extensions
*/


#ifdef TARGET_IS_ANDROID
//#   include <GLES2/gl2.h>
//#   include <GLES2/gl2ext.h>
//#   include "gl3stub.h"





#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#   define myGLGenVertexArrays		glGenVertexArrays
#   define myGLBindVertexArray      glBindVertexArray
#   define myGLDeleteVertexArrays 	glDeleteVertexArrays
#else

#include "GL/gl3w.h"
#include <GLFW/glfw3.h>

GLFWwindow* setupGL(int window_width, int window_height, bool offscreen);
GLFWwindow* createContext(int window_width, int window_height, bool offscreen);
void loadGLExtensions();


#endif



#endif