#ifndef GL_APIS_H
#define GL_APIS_H

#include "gl_setup.h"
#include "misc_headers.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>

struct GPGPUOutputTexture{
    GLuint texture_id;
    GLenum color_attachment;
    int width;
    int height;
    int num_components_per_element;
};


//Setup RGB texture---
GLuint loadRGBTexture(cv::Mat input);

//Setup float textures---
GLuint loadFloatTexture(cv::Mat ip, int num_components_per_element, int width, int height);

//Check for error ---
void checkGLError(std::string func_name);

//Shader stuff---
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
GLint getUniformLocation(GLuint program_ID, std::string unif_name);
GLint getAttribLocation(GLuint program_ID, std::string var_name);

//GPGPU stuff---
GLuint setupFrameBuffer();
GLuint setupQuadVAO(GLuint vert_attribute_id);
void runGPGPU(GLuint fbo, GLuint vao, std::vector<GPGPUOutputTexture>output_textures);
cv::Mat readGPGPUOutputTexture(GLuint fbo, GPGPUOutputTexture t);

#endif