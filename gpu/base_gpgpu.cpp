#include "base_gpgpu.h"
#include <iostream>

BaseGPGPU::BaseGPGPU(){

}

BaseGPGPU::~BaseGPGPU(){

}

void BaseGPGPU::loadQuadVAO(GLuint vert_shader_id){
	//Setup VBO in gpu memory---
	static const float quad_vertices[]={
		-1, -1, 0,
		1, -1, 0,
		-1, 1, 0,
		1, -1, 0,
		1, 1, 0,
		-1, 1, 0		
		// -1, -1, 0,
		// 1, -1, 0,
		// 1, 1, 0		
	};
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	//Create VAO ---
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);


	//Hook up the VBO to attribute in shader---
	glEnableVertexAttribArray(vert_shader_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(
		vert_shader_id,           
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	//Disable VAO 
	glEnableVertexAttribArray(0); 
	glBindVertexArray(0); // Disable our Vertex Array Object  

}

void BaseGPGPU::runGPGPU(){
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	
	// Render to our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0,0,output_texture_width,output_texture_height); 
	
	// Clear the screen
	glClear( GL_COLOR_BUFFER_BIT );

	//Load the VAO
	glBindVertexArray(vao_id);

	// Draw the quad
	glDrawArrays(GL_TRIANGLES, 0, 2*3);
	
	//Unbind the VAO
	glBindVertexArray(0); 

    // Swap buffers
    // glfwSwapBuffers(gl_window);

}

void BaseGPGPU::displayOutputTexture(){
	//Bind the fbo
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//Read back the texture and display in OCV window
	cv::Mat rendered_texture_image = cv::Mat::zeros(output_texture_height,output_texture_width,CV_8UC3);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, output_texture_width, output_texture_height, GL_RGB, GL_UNSIGNED_BYTE,rendered_texture_image.data);
	cv::flip(rendered_texture_image,rendered_texture_image,0);
	cv::cvtColor(rendered_texture_image, rendered_texture_image, CV_RGB2BGR);
	cv::imshow("rendered_texture_image",rendered_texture_image);
	cv::waitKey();
}

void BaseGPGPU::displayFloatOutputTexture(){
	//Bind the fbo
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//Read back the texture and display in OCV window
	cv::Mat rendered_texture_image = cv::Mat::zeros(output_texture_height,output_texture_width,CV_32FC1);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, output_texture_width, output_texture_height, GL_RED, GL_FLOAT,rendered_texture_image.data);
	cv::flip(rendered_texture_image,rendered_texture_image,0);	
	std::cout << "Output texture from gpgpu : " << std::endl << rendered_texture_image << std::endl;
}