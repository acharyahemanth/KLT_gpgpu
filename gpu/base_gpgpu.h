#ifndef BASEGPGPU
#define BASEGPGPU

#include "../common/misc_headers.h"

class BaseGPGPU{
public:
	BaseGPGPU();
	~BaseGPGPU();
	void loadQuadVAO(GLuint vert_shader_id);
//	void runGPGPU();
	void displayOutputTexture();
	void displayFloatOutputTexture();
	GLuint vao_id;
	GLuint fbo, render_texture_ID;
	int output_texture_width, output_texture_height;

};

#endif 