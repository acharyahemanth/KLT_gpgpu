#include "gl_apis.h"

GLuint createGrayTexture(cv::Mat input){
    if(input.empty()){
        std::cout << "ERROR : loadRGBTexture() : Empty mat is passed!" << std::endl;
        return 0;
    }
    
    if(input.channels() != 1){
        std::cout << "ERROR : loadRGBTexture() : # channels != 1" << std::endl;
        return 0;
    }
    
    //Flip the image because uv indexing has its origin at bottom left
    cv::Mat input_flipped = input.clone();
    cv::flip(input, input_flipped, 0);
    
    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0,GL_R8, input_flipped.cols, input_flipped.rows, 0, GL_RED, GL_UNSIGNED_BYTE, input_flipped.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    
    return textureID;
}


GLuint createFloatTexture(cv::Mat ip, int num_components_per_element, int width, int height,
                        GLint texture_filter_method){
    GLenum internal_format, data_format;
    switch(num_components_per_element){
        case 1:
            internal_format = GL_R32F;
            data_format = GL_RED;
            break;
        case 2:
            internal_format = GL_RG32F;
            data_format = GL_RG;
            break;
        case 3:
            internal_format = GL_RGB32F;
            data_format = GL_RGB;
            break;
        case 4:
            internal_format = GL_RGBA32F;
            data_format = GL_RGBA;
            break;
        default:
            std::cout << "ERROR : num_components_per_element >0, <=4" << std::endl;
            return 0;
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    if(!ip.empty()){// Load texture with data
        //Flip the image because uv indexing has its origin at bottom left
        cv::Mat m = ip.clone();
//        cv::flip(m, m, 0);
        glTexImage2D(GL_TEXTURE_2D, 0,internal_format, width, height, 0, data_format, GL_FLOAT, m.data);
    }
    else{// Just setup memory for texture
        glTexImage2D(GL_TEXTURE_2D, 0,internal_format, width, height, 0, data_format, GL_FLOAT, 0);
    }

    //setup up params for texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_filter_method);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_filter_method);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D,0);
    checkGLError("createFloatTexture() ");
    return textureID;
}

//for float, data_format = GL_RED,GL_RG,GL_RGB,GL_RGBA depending on # of components, type = GL_FLOAT
//for rgb data, data_format = GL_RGB, type = GL_UNSIGNED_BYTE
void loadTexture(GLuint texture_id, int x_offset, int y_offset, int width, int height, GLenum data_format, GLenum type, cv::Mat m){
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    x_offset,
                    y_offset,
                    width,
                    height,
                    data_format,
                    type,
                    m.data);
    glBindTexture(GL_TEXTURE_2D,0);
    checkGLError("loadTexture() ");
}

GLuint setupFrameBuffer(){
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    return fbo;
}



//Loads shaders and compiles them into a program. Returns the program ID
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }else{
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }



    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }



    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

GLint getUniformLocation(GLuint program_ID, std::string unif_name) {

	GLint loc = glGetUniformLocation(program_ID, unif_name.c_str());
	if (loc == -1) {
		printf("ERROR : Couldnt find uniform: %s\n", unif_name.c_str());
	} else {
		return (loc);
	}
}

GLint getAttribLocation(GLuint program_ID, std::string var_name) {

	GLint loc = glGetAttribLocation(program_ID, var_name.c_str());
	if (loc == -1) {
		printf("ERROR : Unable to find attrib: %s\n", var_name.c_str());
	} else {
		return (loc);
	}
}

GLuint setupQuadVAO(GLuint vert_attribute_id){
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
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    
    
    //Hook up the VBO to attribute in shader---
    glEnableVertexAttribArray(vert_attribute_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(
                          vert_attribute_id,
                          3,                  // size
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    
    //Disable VAO
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Disable our Vertex Array Object
    
    return vao_id;
    
}

int getMaximumNumberOfColorAttachmentsSupported(){
    GLint maxAttach = 0;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
    
    GLint maxDrawBuf = 0;
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuf);
    
    return std::min(maxAttach, maxDrawBuf);
}

void runGPGPU(GLuint fbo, GLuint vao, std::vector<GPGPUOutputTexture>output_textures){
    if(output_textures.size() > getMaximumNumberOfColorAttachmentsSupported()){
        std::cout << "Error : runGPGPU() : #output textures is greater than whats supported on this system!" << std::endl;
        return;
    }
        
    
    //Attach the textures to fbo attachment points---
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLenum *DrawBuffers = new GLenum[output_textures.size()];
    int smallest_output_texture_width = output_textures[0].width;
    int smallest_output_texture_height = output_textures[0].height;
    for(int i=0;i<output_textures.size();i++){
        glFramebufferTexture(GL_FRAMEBUFFER, output_textures[i].color_attachment, output_textures[i].texture_id, 0);
        DrawBuffers[i] = output_textures[i].color_attachment;
        if(output_textures[i].width < smallest_output_texture_width)
            smallest_output_texture_width = output_textures[i].width;
        if(output_textures[i].height < smallest_output_texture_height)
            smallest_output_texture_height = output_textures[i].height;

    }
    glDrawBuffers(output_textures.size(), DrawBuffers);
    delete DrawBuffers;
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Error : runGPGPU() : tex not attached successfully..." << std::endl;
        return;
    }
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    //Setup viewport to smallest texture
    glViewport(0,0,smallest_output_texture_width,smallest_output_texture_height);
    
    // Clear the screen
    glClear( GL_COLOR_BUFFER_BIT );
    
    //Load the VAO
    glBindVertexArray(vao);
    
    // Draw the quad
    glDrawArrays(GL_TRIANGLES, 0, 2*3);
    
    //Unbind the VAO
    glBindVertexArray(0); 
    
    // Swap buffers
    // glfwSwapBuffers(gl_window);
    
}

cv::Mat readGPGPUOutputTexture(GLuint fbo, GPGPUOutputTexture t){
    //Bind the fbo
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    //Read back the texture and display in OCV window
    cv::Mat gpgpu_output;
    GLenum data_type;
    switch (t.num_components_per_element) {
        case 1:
            gpgpu_output = cv::Mat::zeros(t.height,t.width,CV_32FC1);
            data_type = GL_RED;
            break;
        case 2:
            gpgpu_output = cv::Mat::zeros(t.height,t.width,CV_32FC2);
            data_type = GL_RG;
            break;
        case 3:
            gpgpu_output = cv::Mat::zeros(t.height,t.width,CV_32FC3);
            data_type = GL_RGB;
            break;
        case 4:
            gpgpu_output = cv::Mat::zeros(t.height,t.width,CV_32FC4);
            data_type = GL_RGBA;
            break;
            
        default:
            std::cout << "Error : readGPGPUOutputTexture only supports 4 channel mats!" << std::endl;
            return cv::Mat();
            break;
    }
    
    glReadBuffer(t.color_attachment);
    glReadPixels(0, 0, t.width, t.height, data_type, GL_FLOAT,gpgpu_output.data);
//    cv::flip(gpgpu_output,gpgpu_output,0);
    return gpgpu_output;
}

void checkGLError(
                  std::string func_name
                  ){
    
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) {
        //std::cout << "[PASS] " << func_name << std::endl;
        return;
    } else {
        //std::string err_str = "[FAIL] " + func_name;
        printf("[FAIL GL] %s", func_name.c_str());
        //std::cout << "[FAIL] " << func_name << std::endl;
    }
    
    // check if that worked
    switch(err) {
            
        case GL_NO_ERROR:
            return;
            
        case GL_INVALID_ENUM:
            printf("GL_INVALID_ENUM: GLenum argument out of range");
            //std::cout << "GL_INVALID_ENUM: GLenum argument out of range" << std::endl;
            break;
            
        case GL_INVALID_VALUE:
            printf("GL_INVALID_VALUE: numeric argument out of range");
            //std::cout << "GL_INVALID_VALUE: numeric argument out of range" << std::endl;
            break;
            
        case GL_INVALID_OPERATION:
            printf("GL_INVALID_OPERATION: operation illegal in current state");
            //std::cout << "GL_INVALID_OPERATION: operation illegal in current state" << std::endl;
            break;
            
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            printf("GL_INVALID_FRAMEBUFFER_OPERATION: framebuffer object is not complete");
            //std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION: framebuffer object is not complete" << std::endl;
            break;
            
        case GL_STACK_OVERFLOW:
            std::cout << "GL_STACK_OVERFLOW: "
            "command would cause a stack overflow" << std::endl;
            break;
            
        case GL_STACK_UNDERFLOW:
            std::cout << "GL_STACK_UNDERFLOW: "
            "command would cause a stack underflow" << std::endl;
            break;
            
        case GL_OUT_OF_MEMORY:
            std::cout << "GL_OUT_OF_MEMORY: "
            "not enough memory left to execute command" << std::endl;
            break;
            
        default:
            std::cout << "unlisted error" << std::endl;
            break;
            
    }
    
}
