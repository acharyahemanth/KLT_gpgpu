#ifndef GL_APIS_H
#define GL_APIS_H

#include "misc_headers.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>


//Setup float textures
GLuint initialiseFloatTextureWithCVMat(cv::Mat ip, int num_components_per_element, int width, int height);
void setupFrameBufferAndFloatTexture(int width, int height, GLuint &fbo, GLuint &renderedTexture, int num_components_per_outputelement);

//Check for error 
void checkGLError(std::string func_name);

//Shader stuff
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
GLint getUniformLocation(GLuint program_ID, std::string unif_name);
GLint getAttribLocation(GLuint program_ID, std::string var_name);



#endif