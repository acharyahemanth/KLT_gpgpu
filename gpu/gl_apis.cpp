#include "gl_apis.h"

GLuint initialiseFloatTextureWithCVMat(cv::Mat ip, int num_components_per_element, int width, int height){
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
        cv::flip(m, m, 0);
        glTexImage2D(GL_TEXTURE_2D, 0,internal_format, width, height, 0, data_format, GL_FLOAT, m.data);
    }
    else{// Just setup memory for texture
        glTexImage2D(GL_TEXTURE_2D, 0,internal_format, width, height, 0, data_format, GL_FLOAT, 0);
    }

    //setup up params for texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D,0);
    checkGLError("initialiseFloatTextureWithCVMat");
    return textureID;
}

void setupFrameBufferAndFloatTexture(int width, int height, GLuint &fbo, GLuint &renderedTexture, int num_components_per_outputelement){
    fbo=0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // The texture we're going to render to
    renderedTexture = initialiseFloatTextureWithCVMat(cv::Mat(), num_components_per_outputelement, width, height);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Error : setupFrameBufferAndTexture() : tex not attached successfully..." << std::endl;
        return;
    }

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
		printf("Couldnt find uniform: %s\n", unif_name.c_str());
	} else {
		return (loc);
	}
}

GLint getAttribLocation(GLuint program_ID, std::string var_name) {

	GLint loc = glGetAttribLocation(program_ID, var_name.c_str());
	if (loc == -1) {
		printf("error in attrib: %s\n", var_name.c_str());
	} else {
		return (loc);
	}
}
