#ifndef GL_APIS_H
#define GL_APIS_H

#include "../common/misc_headers.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <string.h>

#ifndef TARGET_IS_ANDROID
#   define SHADER_HEADER \
        "#version 330 core\n"
#else
#   define SHADER_HEADER \
        "#version 300 es \n"
#endif


struct GPGPUOutputTexture{
    GLuint texture_id;
    GLenum color_attachment;
    int width;
    int height;
    int num_components_per_element;
};


//Setup RGB texture---
GLuint createGrayTexture(cv::Mat input);

//Setup float textures---
GLuint createFloatTexture(cv::Mat ip, int num_components_per_element, int width, int height,
                        GLint texture_filter_method=GL_NEAREST);

//Load data into an exisiting texture---
void loadTexture(GLuint texture_id, int x_offset, int y_offset, int width, int height, GLenum data_format, GLenum type, cv::Mat m);

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