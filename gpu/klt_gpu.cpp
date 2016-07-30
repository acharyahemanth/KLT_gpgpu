#include "klt_gpu.h"

KLT_gpu::KLT_gpu(int num_pyramid_levels, int window_size){
    this->num_pyramid_levels = num_pyramid_levels;
    assert(window_size%2!=0);
    this->window_size = window_size;
    
    //Constants
    num_iterations_kl_tracker = 20;
    min_displacement_exit_criterion_kl_tracker = 1e-4;
    
    //Shader stuff---
    //getA.fsh
//    std::string vs = std::string(BASE_TEST_DIR) + "/shaders/gpgpu_quad.vsh";
//    std::string fs = std::string(BASE_TEST_DIR) + "/shaders/getA.fsh";
//    getA_shader_id = LoadShaders( vs.c_str(), fs.c_str() );
//    getA_sh_vert_id = getAttribLocation(getA_shader_id, "vert");
//    getA_sh_srcimage_texture_sampler_id = getUniformLocation(getA_shader_id, "srcimage_texture_sampler");
//    getA_sh_srcpts_texture_sampler_id = getUniformLocation(getA_shader_id, "srcpts_texture_sampler");
//    getA_sh_num_points_id = getUniformLocation(getA_shader_id, "num_points");
//    getA_sh_window_size_id = getUniformLocation(getA_shader_id, "window_size");
//    getA_sh_image_width_id = getUniformLocation(getA_shader_id, "image_width");
//    getA_sh_image_height_id = getUniformLocation(getA_shader_id, "image_height");
//    getA_sh_vao_id = setupQuadVAO(getA_sh_vert_id);
    
    //dbgshader
    std::string vs = std::string(BASE_TEST_DIR) + "/shaders/gpgpu_quad.vsh";
    std::string fs = std::string(BASE_TEST_DIR) + "/shaders/dbg_shader.fsh";
    dbg_sh_id = LoadShaders( vs.c_str(), fs.c_str() );
    dbg_sh_vert_id = getAttribLocation(dbg_sh_id, "vert");
    dbg_sh_ip_texture_sampler = getUniformLocation(dbg_sh_id, "ip_texture_sampler");
    dbg_sh_vao_id = setupQuadVAO(dbg_sh_vert_id);
    
    //Frame buffer that we will be rendering to---
    fbo_id = setupFrameBuffer();

}
KLT_gpu::~KLT_gpu(){
    
}
void KLT_gpu::execute_dbg(){
    int w=3;
    int h=3;
    
    //setup input texture
    cv::Mat ip=cv::Mat::ones(h,w,CV_8UC1);
    ip.convertTo(ip, CV_32FC1);
    for(int r=0;r<h;r++){
        for(int c=0;c<w;c++){
            ip.at<float>(r,c) = r*10+c;
        }
    }
    GLuint input_texture_id = loadFloatTexture(ip,
                                               1,
                                               w,
                                               h);
    
    //setup output texture
    std::vector<GPGPUOutputTexture>outputs(2);
    for(int i=0;i<outputs.size();i++){
        outputs[i].width = w;
        outputs[i].height = h;
        outputs[i].num_components_per_element = 1;
        outputs[i].texture_id  = loadFloatTexture(cv::Mat(),
                                                  1,
                                                  outputs[i].width,
                                                  outputs[i].height);
    }
    outputs[0].color_attachment = GL_COLOR_ATTACHMENT0;
    outputs[1].color_attachment = GL_COLOR_ATTACHMENT1;
    
    glUseProgram(dbg_sh_id);
    
    //Hook up tex and sampler
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input_texture_id);//bind texture
    glUniform1i(dbg_sh_ip_texture_sampler, 0);//bind sampler

    
    runGPGPU(fbo_id,
             dbg_sh_vao_id,
             outputs);
    
    cv::Mat a = readGPGPUOutputTexture(fbo_id, outputs[0]);
    cv::Mat b = readGPGPUOutputTexture(fbo_id, outputs[1]);
    std::cout << "GPGPU output 1 : " << std::endl << a << std::endl;
    std::cout << "GPGPU output 2 : " << std::endl << b << std::endl;
}

void KLT_gpu::execute(cv::Mat source,
                      cv::Mat dest,
                      std::vector<cv::Point2f>src_pts,
                      std::vector<cv::Point2f>&tracked_pts,
                      std::vector<bool>&error){
    
    source_image_width = source.cols;
    source_image_height = source.rows;
    
    //project src_pts to top level of pyr---
    for(int i=0;i<src_pts.size();i++){
        src_pts[i] = src_pts[i] / (1<<(num_pyramid_levels-1));
    }
    total_number_points_being_tracked = src_pts.size();
    
    //Load input data into textures---
    texture_pairs_ppong=0;
    loadTexturesWithData(source,
                         dest,
                         src_pts,
                         src_pts);
    
    //Start from topmost layer and calc optical flow for all pts
    for(int l=num_pyramid_levels-1; l>=0; l--){
        iterativeTrackerAtAPyramidLevel(l);
        
        //project src_pts+tracked_pts to next level
//        projectPointsToNextLevel();
    }
}

void KLT_gpu::iterativeTrackerAtAPyramidLevel(int pyramid_level){
    
    //Evaluate the A matrix for all points---
    calcA(pyramid_level);
    
    //Go through iterations for all points---
//    for(int k=0;k<num_iterations_kl_tracker;k++){
//        //Calculate the b matrix for all points---
//        calcb();
//        
//        //Calculate tracked point for all points---
//        track();
//        
//        //Copy prediction texture from output to input---
//    }

}

void KLT_gpu::loadTexturesWithData(cv::Mat source,
                                   cv::Mat dest,
                                   std::vector<cv::Point2f>source_points,
                                   std::vector<cv::Point2f>prediction){
    //dbg
    std::cout << "Source points are -> " << std::endl;
    for(int i=0;i<source_points.size();i++){
        std::cout << source_points[i] << std::endl;
    }
    std::cout << "src image pel :  " << (int)source.at<uchar>(150,150) << std::endl;
    
    source_image_id = loadGrayTexture(source);
    dest_image_id = loadGrayTexture(dest);
    cv::Mat source_pts_mat = cv::Mat::zeros(1, source_points.size(), CV_32FC2);
    cv::Mat prediction_pts_mat = cv::Mat::zeros(1, prediction.size(), CV_32FC2);
    for(int i=0;i<source_points.size();i++){
        source_pts_mat.at<cv::Vec2f>(0,i) = cv::Vec2f(source_points[i].x,
                                                      source_points[i].y);
        prediction_pts_mat.at<cv::Vec2f>(0,i) = cv::Vec2f(prediction[i].x,
                                                      prediction[i].y);
    }
    source_points_id = loadFloatTexture(source_pts_mat,
                                        2,
                                        source_points.size(),
                                        1);
    prediction_points_id[texture_pairs_ppong] = loadFloatTexture(source_pts_mat,
                                        2,
                                        source_points.size(),
                                        1);
    prediction_points_id[(texture_pairs_ppong+1)%2] = loadFloatTexture(cv::Mat(),
                                                                 2,
                                                                 source_points.size(),
                                                                 1);

}

void KLT_gpu::calcA(int pyramid_level){
    //Use the getA shader
    glUseProgram(getA_shader_id);
    
    //Setup output textures
    getA_sh_Amat_output.width = total_number_points_being_tracked;
    getA_sh_Amat_output.height = window_size*window_size;
    getA_sh_Amat_output.num_components_per_element=2;
    getA_sh_Amat_output.texture_id  = loadFloatTexture(cv::Mat(),
                                           2,
                                           getA_sh_Amat_output.width,
                                           getA_sh_Amat_output.height);
    for(int i=0;i<2;i++){
        getA_sh_W_output[i].width = total_number_points_being_tracked;
        getA_sh_W_output[i].height = window_size*window_size;
        getA_sh_W_output[i].num_components_per_element=1;
        getA_sh_W_output[i].texture_id = loadFloatTexture(cv::Mat(),
                                               1,
                                               getA_sh_W_output[i].width,
                                               getA_sh_W_output[i].height);
    }
    
    //Update shader variables and input textures---
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source_image_id);
    glUniform1i(getA_sh_srcimage_texture_sampler_id, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, source_points_id);
    glUniform1i(getA_sh_srcpts_texture_sampler_id, 1);
    
	glUniform1i(getA_sh_num_points_id, total_number_points_being_tracked);
	glUniform1i(getA_sh_window_size_id, window_size);
	glUniform1i(getA_sh_image_width_id, source_image_width);
	glUniform1i(getA_sh_image_height_id, source_image_height);
    
    //Run shader
    std::vector<GPGPUOutputTexture>outputs(2);
    outputs[0] = getA_sh_Amat_output;
    outputs[0].color_attachment = GL_COLOR_ATTACHMENT0;
    outputs[1] = getA_sh_W_output[texture_pairs_ppong];
    outputs[1].color_attachment = GL_COLOR_ATTACHMENT1;
    
    runGPGPU(fbo_id, getA_sh_vao_id, outputs);
    
    //Read back shader calculation for debug
    cv::Mat A = readGPGPUOutputTexture(fbo_id, outputs[0]);
    cv::Mat w = readGPGPUOutputTexture(fbo_id, outputs[1]);
    std::cout << "A mat -> " << std::endl << A << std::endl;
    std::cout << "W mat -> " << std::endl << w << std::endl;
}