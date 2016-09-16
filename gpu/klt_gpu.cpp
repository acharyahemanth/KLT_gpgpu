#include "klt_gpu.h"

KLT_gpu::KLT_gpu(int num_pyramid_levels, int window_size, int image_width, int image_height){
    this->num_pyramid_levels = num_pyramid_levels;
    assert(window_size%2!=0);
    this->window_size = window_size;
    source_image_width = image_width;
    source_image_height = image_height;

    
    //Constants
    num_iterations_kl_tracker = 10;//TODO : need to bail out earlier if all points have been tracked!
    min_displacement_exit_criterion_kl_tracker = 1e-2;
    max_number_of_points_supported = 300;
    margin_to_declare_tracking_lost = 1;
    
    //Shader stuff---
    //getA.fsh
    std::string vs = std::string(BASE_TEST_DIR) + "shaders/gpgpu_quad.vsh";
    std::string fs = std::string(BASE_TEST_DIR) + "shaders/getA.fsh";
    getA_shader_id = LoadShaders( vs.c_str(), fs.c_str() );
    getA_sh_vert_id = getAttribLocation(getA_shader_id, "vert");
    getA_sh_srcimage_texture_sampler_id = getUniformLocation(getA_shader_id, "srcimage_texture_sampler");
    getA_sh_srcpts_texture_sampler_id = getUniformLocation(getA_shader_id, "srcpts_texture_sampler");
    getA_sh_num_points_id = getUniformLocation(getA_shader_id, "num_points");
    getA_sh_window_size_id = getUniformLocation(getA_shader_id, "window_size");
    getA_sh_image_width_id = getUniformLocation(getA_shader_id, "image_width");
    getA_sh_image_height_id = getUniformLocation(getA_shader_id, "image_height");
    getA_sh_pyramid_level_id = getUniformLocation(getA_shader_id, "pyramid_level");
    getA_sh_vao_id = setupQuadVAO(getA_sh_vert_id);

    //getb.fsh
    vs = std::string(BASE_TEST_DIR) + "shaders/gpgpu_quad.vsh";
    fs = std::string(BASE_TEST_DIR) + "shaders/getb.fsh";
    getb_shader_id = LoadShaders( vs.c_str(), fs.c_str() );
    getb_sh_vert_id = getAttribLocation(getb_shader_id, "vert");
    getb_sh_srcimage_texture_sampler_id = getUniformLocation(getb_shader_id, "srcimage_texture_sampler");
    getb_sh_dstimage_texture_sampler_id = getUniformLocation(getb_shader_id, "dstimage_texture_sampler");
    getb_sh_srcpts_texture_sampler_id = getUniformLocation(getb_shader_id, "srcpts_texture_sampler");
    getb_sh_predpts_texture_sampler_id = getUniformLocation(getb_shader_id, "predpts_texture_sampler");
    getb_sh_num_points_id = getUniformLocation(getb_shader_id, "num_points");
    getb_sh_window_size_id = getUniformLocation(getb_shader_id, "window_size");
    getb_sh_image_width_id = getUniformLocation(getb_shader_id, "image_width");
    getb_sh_image_height_id = getUniformLocation(getb_shader_id, "image_height");
    getb_sh_pyramid_level_id = getUniformLocation(getb_shader_id, "pyramid_level");
    getb_sh_vao_id = setupQuadVAO(getb_sh_vert_id);

    
    //track.fsh
    vs = std::string(BASE_TEST_DIR) + "shaders/gpgpu_quad.vsh";
    fs = std::string(BASE_TEST_DIR) + "shaders/track.fsh";
    track_shader_id = LoadShaders( vs.c_str(), fs.c_str() );
    track_sh_vert_id = getAttribLocation(track_shader_id, "vert");
    track_sh_A_texture_sampler_id = getUniformLocation(track_shader_id, "A_texture_sampler");
    track_sh_b_texture_sampler_id = getUniformLocation(track_shader_id, "b_texture_sampler");
    track_sh_predpts_texture_sampler_id = getUniformLocation(track_shader_id, "predpts_texture_sampler");
    track_sh_num_points_id = getUniformLocation(track_shader_id, "num_points");
    track_sh_window_size_id = getUniformLocation(track_shader_id, "window_size");
    track_sh_image_width_id = getUniformLocation(track_shader_id, "image_width");
    track_sh_image_height_id = getUniformLocation(track_shader_id, "image_height");
    track_sh_pyramid_level_id = getUniformLocation(track_shader_id, "pyramid_level");
    track_sh_vao_id = setupQuadVAO(track_sh_vert_id);
    
    //next_level.fsh
    vs = std::string(BASE_TEST_DIR) + "shaders/gpgpu_quad.vsh";
    fs = std::string(BASE_TEST_DIR) + "shaders/next_level.fsh";
    next_level_shader_id = LoadShaders( vs.c_str(), fs.c_str() );
    next_level_sh_vert_id = getAttribLocation(next_level_shader_id, "vert");
    next_level_sh_srcpts_texture_sampler_id = getUniformLocation(next_level_shader_id, "srcpts_texture_sampler");
    next_level_sh_predpts_texture_sampler_id = getUniformLocation(next_level_shader_id, "predpts_texture_sampler");
    next_level_sh_num_points_id = getUniformLocation(next_level_shader_id, "num_points");
    next_level_sh_vao_id = setupQuadVAO(next_level_sh_vert_id);
    
    
    //back_image_shader
    vs = std::string(BASE_TEST_DIR) + "shaders/gpgpu_quad.vsh";
    fs = std::string(BASE_TEST_DIR) + "shaders/back_image.fsh";
    back_image_shader_id = LoadShaders( vs.c_str(), fs.c_str() );
    back_image_sh_vert_id = getAttribLocation(back_image_shader_id, "vert");
    back_image_sh_srcimage_texture_sampler_id = getUniformLocation(back_image_shader_id, "srcimage_texture_sampler");
    back_image_sh_image_width_id = getUniformLocation(back_image_shader_id, "image_width");
    back_image_sh_image_height_id = getUniformLocation(back_image_shader_id, "image_height");
    back_image_sh_vao_id = setupQuadVAO(back_image_sh_vert_id);

    
    //dbgshader
//    std::string vs = std::string(BASE_TEST_DIR) + "shaders/gpgpu_quad.vsh";
//    std::string fs = std::string(BASE_TEST_DIR) + "shaders/dbg_shader.fsh";
//    dbg_sh_id = LoadShaders( vs.c_str(), fs.c_str() );
//    dbg_sh_vert_id = getAttribLocation(dbg_sh_id, "vert");
//    dbg_sh_ip_texture_sampler = getUniformLocation(dbg_sh_id, "ip_texture_sampler");
//    dbg_sh_vao_id = setupQuadVAO(dbg_sh_vert_id);
    
    //Frame buffer that we will be rendering to---
    fbo_id = setupFrameBuffer();
    
    //Reserve space in the GPU for all textures---
    setupTextures();
}
KLT_gpu::~KLT_gpu(){
    myLOGD("~KLT_gpu()");
#ifndef TARGET_IS_ANDROID
    glDeleteTextures(1, &getb_sh_bmat_output.texture_id);
    glDeleteTextures(1, &getA_sh_Amat_output.texture_id);
    for(int i=0;i<2;i++){
        glDeleteTextures(1,&next_sh_source_points_output[i].texture_id);
        glDeleteTextures(1,&track_sh_prediction_output[i].texture_id);
    }
    glDeleteTextures(1,&source_image_id);
    glDeleteTextures(1,&dest_image_id);
#endif
}

void KLT_gpu::setupTextures(){
    //output of getA.fsh
    getA_sh_Amat_output.width = max_number_of_points_supported;
    getA_sh_Amat_output.height = window_size*window_size;
    getA_sh_Amat_output.num_components_per_element=2;
    getA_sh_Amat_output.texture_id  = createFloatTexture(cv::Mat(),
                                                       2,
                                                       getA_sh_Amat_output.width,
                                                       getA_sh_Amat_output.height);
    
    //output of getb.fsh
    getb_sh_bmat_output.width = max_number_of_points_supported;
    getb_sh_bmat_output.height = window_size*window_size;
    getb_sh_bmat_output.num_components_per_element=1;
    getb_sh_bmat_output.texture_id  = createFloatTexture(cv::Mat(),
                                                       getb_sh_bmat_output.num_components_per_element,
                                                       getb_sh_bmat_output.width,
                                                       getb_sh_bmat_output.height);
    
    //input / outputs
    for(int i=0;i<2;i++){
        next_sh_source_points_output[i].width = max_number_of_points_supported;
        next_sh_source_points_output[i].height = 1;
        next_sh_source_points_output[i].num_components_per_element = 2;
        next_sh_source_points_output[i].texture_id = createFloatTexture(cv::Mat(),
                                                                        2,
                                                                        next_sh_source_points_output[i].width,
                                                                        1);
        
        track_sh_prediction_output[i].width = max_number_of_points_supported;
        track_sh_prediction_output[i].height = 1;
        track_sh_prediction_output[i].num_components_per_element = 2;
        track_sh_prediction_output[i].texture_id = createFloatTexture(cv::Mat(),
                                                                      2,
                                                                      track_sh_prediction_output[i].width,
                                                                      1);
    }
//  output of track.fsh
    point_shift_delta.width = max_number_of_points_supported;
    point_shift_delta.height = 1;
    point_shift_delta.num_components_per_element = 1;
    point_shift_delta.texture_id = createFloatTexture(cv::Mat(),
                                                     point_shift_delta.num_components_per_element,
                                                     point_shift_delta.width,
                                                     point_shift_delta.height);
    
    
    //input images
    source_image_id = createFloatTexture(cv::Mat(), 1, source_image_width, source_image_height);//GLES doesnt like this : , GL_LINEAR);
    dest_image_id = createFloatTexture(cv::Mat(), 1, source_image_width, source_image_height);// GL_LINEAR);
    
    //display back buffer
    back_image_id = createRGBTexture(source_image_width, source_image_height);
}

void KLT_gpu::execute(cv::Mat source,
                      cv::Mat dest,
                      std::vector<cv::Point2f>src_pts,
                      std::vector<cv::Point2f>&tracked_pts,
                      std::vector<bool>&error){
    
    //project src_pts to top level of pyr---
    for(int i=0;i<src_pts.size();i++){
        src_pts[i] = src_pts[i] / (1<<(num_pyramid_levels-1));
    }
    total_number_points_being_tracked = src_pts.size();
    
    //Load input data into textures---
    ppong_idx_iterations=0;
    ppong_idx_pyramid_level=0;
    loadTexturesWithData(source,
                         dest,
                         src_pts,
                         src_pts);
    
    //Start from topmost layer and calc optical flow for all pts
    for(int l=num_pyramid_levels-1; l>=0; l--){
//        std::cout << "Processing pyr level " << l << " ..." << std::endl;
        
        //Track points at current pyramid level
        iterativeTrackerAtAPyramidLevel(l);
        
        //No need to project points to next level :: final result will be in ppong_idx_iterations
        if(l==0){
            break;
        }
        
        //project src_pts+tracked_pts to next level
        projectPointsToNextLevel();
        
        //update ppong buffers
        ppong_idx_iterations = (ppong_idx_iterations+1)%2;
        ppong_idx_pyramid_level = (ppong_idx_pyramid_level+1)%2;
    }
    
    //Populate the output data structures---
    populateOutputDS(tracked_pts, error);
}

void KLT_gpu::iterativeTrackerAtAPyramidLevel(int pyramid_level){
    
    //Evaluate the A matrix for all points---
    calcA(pyramid_level);
    
    //Go through iterations for all points---
    for(int k=0;k<num_iterations_kl_tracker;k++){
//        std::cout << "Processing iteration #" << k << std::endl;
        //Calculate the b matrix for all points---
        calcb(pyramid_level);

        //Calculate tracked point for all points---
        track(pyramid_level);

        //Check if any more iterations are reqd---
//        bool is_next_iteration_reqd = isNextIterationReqd();

        //Swap ping pong buffers for next iteration---
        ppong_idx_iterations = (ppong_idx_iterations+1)%2;
    }
}

void KLT_gpu::loadTexturesWithData(cv::Mat source,
                                   cv::Mat dest,
                                   std::vector<cv::Point2f>source_points,
                                   std::vector<cv::Point2f>prediction){
    source.convertTo(source, CV_32FC1);
    dest.convertTo(dest, CV_32FC1);
    loadTexture(source_image_id, 0, 0, source_image_width, source_image_height, GL_RED, GL_FLOAT, source);
    loadTexture(dest_image_id, 0, 0, source_image_width, source_image_height, GL_RED, GL_FLOAT, dest);
    cv::Mat source_pts_mat = cv::Mat::zeros(1, source_points.size(), CV_32FC2);
    cv::Mat prediction_pts_mat = cv::Mat::zeros(1, prediction.size(), CV_32FC2);
    for(int i=0;i<source_points.size();i++){
        source_pts_mat.at<cv::Vec2f>(0,i) = cv::Vec2f(source_points[i].x,
                                                      source_points[i].y);
        prediction_pts_mat.at<cv::Vec2f>(0,i) = cv::Vec2f(prediction[i].x,
                                                      prediction[i].y);
    }
    for(int i=0;i<2;i++){
        loadTexture(next_sh_source_points_output[i].texture_id, 0, 0, next_sh_source_points_output[i].width, next_sh_source_points_output[i].height, GL_RG, GL_FLOAT, source_pts_mat);
        loadTexture(track_sh_prediction_output[i].texture_id, 0, 0, track_sh_prediction_output[i].width, track_sh_prediction_output[i].height, GL_RG, GL_FLOAT, prediction_pts_mat);

    }
}

void KLT_gpu::calcA(int pyramid_level){
    //Use the getA shader
    glUseProgram(getA_shader_id);
    
    //Setup output textures
    
    //Update shader variables and input textures---
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source_image_id);
    glUniform1i(getA_sh_srcimage_texture_sampler_id, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, next_sh_source_points_output[ppong_idx_pyramid_level].texture_id);
    glUniform1i(getA_sh_srcpts_texture_sampler_id, 1);
    
	glUniform1i(getA_sh_num_points_id, max_number_of_points_supported);
	glUniform1i(getA_sh_window_size_id, window_size);
	glUniform1i(getA_sh_image_width_id, source_image_width);
	glUniform1i(getA_sh_image_height_id, source_image_height);
	glUniform1i(getA_sh_pyramid_level_id, pyramid_level);
    
    //Run shader
    std::vector<GPGPUOutputTexture>outputs(1);
    outputs[0] = getA_sh_Amat_output;
    outputs[0].color_attachment = GL_COLOR_ATTACHMENT0;
    
    runGPGPU(fbo_id, getA_sh_vao_id, outputs);
    
    //Read back shader calculation for debug
//    cv::Mat A = readGPGPUOutputTexture(fbo_id, outputs[0]);
//    std::cout << "A mat -> " << std::endl << A.col(0) << std::endl;
//    myLOGD("A mat -> %s",printMatString(A.col(3)).c_str());
}

void KLT_gpu::calcb(int pyramid_level){
    //Use the getb shader
    glUseProgram(getb_shader_id);
    
    //Setup output textures
    
    //Update shader variables and input textures---
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source_image_id);
    glUniform1i(getb_sh_srcimage_texture_sampler_id, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, dest_image_id);
    glUniform1i(getb_sh_dstimage_texture_sampler_id, 1);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, next_sh_source_points_output[ppong_idx_pyramid_level].texture_id);
    glUniform1i(getb_sh_srcpts_texture_sampler_id, 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, track_sh_prediction_output[ppong_idx_iterations].texture_id);
    glUniform1i(getb_sh_predpts_texture_sampler_id, 3);
    
    
    glUniform1i(getb_sh_num_points_id, max_number_of_points_supported);
    glUniform1i(getb_sh_window_size_id, window_size);
    glUniform1i(getb_sh_image_width_id, source_image_width);
    glUniform1i(getb_sh_image_height_id, source_image_height);
    glUniform1i(getb_sh_pyramid_level_id, pyramid_level);
    
    //Run shader
    std::vector<GPGPUOutputTexture>outputs(1);
    outputs[0] = getb_sh_bmat_output;
    outputs[0].color_attachment = GL_COLOR_ATTACHMENT0;
    
    runGPGPU(fbo_id, getb_sh_vao_id, outputs);
    
    //Read back shader calculation for debug
//    cv::Mat b = readGPGPUOutputTexture(fbo_id, outputs[0]);
//    std::cout << "b mat -> " << std::endl << b << std::endl;
//    myLOGD("b mat -> %s",printMatString(b.col(3)).c_str());
}

void KLT_gpu::track(int pyramid_level){
    //Use the track shader
    glUseProgram(track_shader_id);
    
    //Setup output textures
    
    //Update shader variables and input textures---
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, track_sh_prediction_output[ppong_idx_iterations].texture_id);
    glUniform1i(track_sh_predpts_texture_sampler_id, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, getA_sh_Amat_output.texture_id);
    glUniform1i(track_sh_A_texture_sampler_id, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, getb_sh_bmat_output.texture_id);
    glUniform1i(track_sh_b_texture_sampler_id, 2);

    
    glUniform1i(track_sh_num_points_id, max_number_of_points_supported);
    glUniform1i(track_sh_window_size_id, window_size);
    glUniform1i(track_sh_image_width_id, source_image_width);
    glUniform1i(track_sh_image_height_id, source_image_height);
    glUniform1i(track_sh_pyramid_level_id, pyramid_level);
    
    //Run shader
    std::vector<GPGPUOutputTexture>outputs(2);
    outputs[0] = track_sh_prediction_output[(ppong_idx_iterations+1)%2];
    outputs[0].color_attachment = GL_COLOR_ATTACHMENT0;
    outputs[1] = point_shift_delta;
    outputs[1].color_attachment = GL_COLOR_ATTACHMENT1;

    
    runGPGPU(fbo_id, track_sh_vao_id, outputs);
    
    //Read back shader calculation for debug
//    cv::Mat track_pts = readGPGPUOutputTexture(fbo_id, outputs[0]);
//    std::cout << "track_pts mat -> " << std::endl << track_pts << std::endl;
}

void KLT_gpu::projectPointsToNextLevel(){
    //Use the track shader
    glUseProgram(next_level_shader_id);
    
    //Setup output textures
    
    //Update shader variables and input textures---
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, next_sh_source_points_output[ppong_idx_pyramid_level].texture_id);
    glUniform1i(next_level_sh_srcpts_texture_sampler_id, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, track_sh_prediction_output[ppong_idx_iterations].texture_id);
    glUniform1i(next_level_sh_predpts_texture_sampler_id, 1);
    
    
    
    glUniform1i(next_level_sh_num_points_id, max_number_of_points_supported);
    
    //Run shader
    std::vector<GPGPUOutputTexture>outputs(2);
    outputs[0] = next_sh_source_points_output[(ppong_idx_pyramid_level+1)%2];
    outputs[0].color_attachment = GL_COLOR_ATTACHMENT0;
    outputs[1] = track_sh_prediction_output[(ppong_idx_iterations+1)%2];
    outputs[1].color_attachment = GL_COLOR_ATTACHMENT1;
    
    
    runGPGPU(fbo_id, next_level_sh_vao_id, outputs);
    
    //Read back shader calculation for debug
//    cv::Mat src_pts = readGPGPUOutputTexture(fbo_id, outputs[0]);
    //    cv::Mat prediction_pts = readGPGPUOutputTexture(fbo_id, outputs[1]);
    //    std::cout << "For next level, src_pts mat -> " << std::endl << src_pts << std::endl;
    //    std::cout << "For next level, prediction_pts mat -> " << std::endl << prediction_pts << std::endl;
}

void KLT_gpu::populateOutputDS(std::vector<cv::Point2f> &tracked_pts, std::vector<bool> &error){
    //Attach the final prediction buffer to fbo
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
//    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, track_sh_prediction_output[ppong_idx_iterations].texture_id, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           track_sh_prediction_output[ppong_idx_iterations].texture_id, 0);


    //Read the tracked points from the final prediction texture
    std::vector<GPGPUOutputTexture>outputs(1);
    outputs[0] = track_sh_prediction_output[ppong_idx_iterations];
    outputs[0].color_attachment = GL_COLOR_ATTACHMENT0;
    cv::Mat track_pts_mat = readGPGPUOutputTexture(fbo_id, outputs[0]);
    
    tracked_pts.clear();
    error.clear();
    tracked_pts.resize(total_number_points_being_tracked);
    error.resize(total_number_points_being_tracked,false);//TODO : fix this later
    for(int i=0;i<tracked_pts.size();i++){
        tracked_pts[i].x = track_pts_mat.at<cv::Vec2f>(0,i).val[0];
        tracked_pts[i].y = track_pts_mat.at<cv::Vec2f>(0,i).val[1];
        
        if(tracked_pts[i].x < margin_to_declare_tracking_lost ||
           tracked_pts[i].y < margin_to_declare_tracking_lost ||
           tracked_pts[i].x >= source_image_width - margin_to_declare_tracking_lost ||
           tracked_pts[i].y >= source_image_height - margin_to_declare_tracking_lost){
            error[i] = true;
        }
    }
    
}
//
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
    GLuint input_texture_id = createFloatTexture(ip,
                                               1,
                                               w,
                                               h);
    
    //setup output texture
    std::vector<GPGPUOutputTexture>outputs(2);
    for(int i=0;i<outputs.size();i++){
        outputs[i].width = w;
        outputs[i].height = h;
        outputs[i].num_components_per_element = 1;
        outputs[i].texture_id  = createFloatTexture(cv::Mat(),
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

void KLT_gpu::drawFrame(cv::Mat img, int screen_width, int screen_height, std::vector<cv::Point2f>tracked_corners, std::vector<bool>error){
    //Use the back_image shader
    glUseProgram(back_image_shader_id);

    //Draw tracked corners
    for(int i=0;i<tracked_corners.size();i++){
        if(!error[i]){
            cv::circle(img, tracked_corners[i], 2, cv::Scalar(0,255,0),cv::FILLED);
        }
    }
    cv::flip(img, img, 0);

    //Load input image into texture
    loadTexture(back_image_id,
                0,
                0,
                source_image_width,
                source_image_height,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                img);
    
    //Update shader variables and input textures---
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, back_image_id);
    glUniform1i(back_image_sh_srcimage_texture_sampler_id, 0);
    
        
    glUniform1i(back_image_sh_image_width_id, screen_width);
    glUniform1i(back_image_sh_image_height_id, screen_height);
    
    //Run shader
    myLOGD("rendering to screen");
    renderToScreen(back_image_sh_vao_id, screen_width, screen_height);
}

std::string KLT_gpu::printMatString(cv::Mat m){
    std::stringstream ss;
    ss << m;
    return ss.str();
}

void KLT_gpu::execute_ocv(cv::Mat source,
                          cv::Mat dest,
                          std::vector<cv::Point2f>src_pts,
                          std::vector<cv::Point2f>&tracked_pts,
                          std::vector<bool>&error){
    tracked_pts.clear();
    error.clear();

    if(src_pts.size() == 0)
        return;

    std::vector<unsigned char>status;
    cv::Mat err_mat;
    tracked_pts.resize(src_pts.size());
    error.resize(src_pts.size(),true);
    cv::calcOpticalFlowPyrLK(source,
                             dest,
                             src_pts,
                             tracked_pts,
                             status,
                             err_mat,
                             cv::Size(window_size,window_size),
                             num_pyramid_levels);
    for(int i=0;i<src_pts.size();i++){
        if(status[i])
            error[i] = false;
    }
}

bool KLT_gpu::isNextIterationReqd(){
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           point_shift_delta.texture_id, 0);
    
    
    //Read the is_pt_tracked
    std::vector<GPGPUOutputTexture>outputs(1);
    outputs[0] = point_shift_delta;
    outputs[0].color_attachment = GL_COLOR_ATTACHMENT0;
    cv::Mat is_points_tracked_mat = readGPGPUOutputTexture(fbo_id, outputs[0]);
    int num_tracked_pts=0;
    for(int i=0;i<total_number_points_being_tracked;i++){
        if(is_points_tracked_mat.at<float>(0,i) < min_displacement_exit_criterion_kl_tracker)
            num_tracked_pts++;
    }
    std::cout << "# pts which are done : " << num_tracked_pts << std::endl;
    if(num_tracked_pts == total_number_points_being_tracked){
        std::cout << "All points are tracked! No more iterations reqd!" << std::endl;
        return true;
    }
    return false;
}

