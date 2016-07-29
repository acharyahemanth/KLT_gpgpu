#include "misc_headers.h"
#include <iostream>

GLFWwindow* setupGL(int window_width, int window_height, bool off_screen){
	GLFWwindow* window = createContext( window_width,  window_height,  off_screen);
	loadGLExtensions();	
    return window;
}

GLFWwindow* createContext(int window_width, int window_height, bool off_screen){
	GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit()){
    	std::cout << "ERROR : glfw initialisation failed!" << std::endl;
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.2
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 


    if (off_screen == true) {
        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
    }


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "GL_window", NULL, NULL);
    if (!window)
    {
    	std::cout << "Unable to create window..." << std::endl;
        glfwTerminate();
        return NULL;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    printf("GLFW version received: %d.%d.%d\n", major, minor, rev);

    return window;
}

void loadGLExtensions(){
	// initialize gl3w
    if (gl3wInit()) {
        std::cout << "failed to initialize OpenGL" << std::endl;
        // throw GLException("Failed to initialize OpenGL", ERROR_GL_INIT);
    }

}
