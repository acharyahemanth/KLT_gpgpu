#ifndef KLTGPUH
#define KLTGPUH

#include "gl_apis.h"

class KLT_gpu{
public:
    KLT_gpu(int num_pyramid_levels, int window_size);
    ~KLT_gpu();
    void execute(cv::Mat source,
                 cv::Mat dest,
                 std::vector<cv::Point2f>src_pts,
                 std::vector<cv::Point2f>&tracked_pts,
                 std::vector<bool>&error);
    void execute_dbg();
private:
    void iterativeTrackerAtAPyramidLevel(int pyramid_level);
    void loadTexturesWithData(cv::Mat source,
                              cv::Mat dest,
                              std::vector<cv::Point2f>source_points,
                              std::vector<cv::Point2f>prediction);
    void calcA(int pyramid_level);
                              
    //Shader variables
    GLuint source_image_id, dest_image_id, source_points_id, getA_shader_id;
    GLuint prediction_points_id[2];
    GLuint getA_sh_srcimage_texture_sampler_id, getA_sh_srcpts_texture_sampler_id;
    GLuint getA_sh_num_points_id, getA_sh_window_size_id, getA_sh_image_width_id, getA_sh_image_height_id, getA_sh_vao_id, getA_sh_vert_id;
    int texture_pairs_ppong;
    GLuint fbo_id;
    GLuint dbg_sh_id, dbg_sh_vert_id, dbg_sh_vao_id;
    
    //Output textures
    GPGPUOutputTexture getA_sh_Amat_output;
    GPGPUOutputTexture getA_sh_W_output[2];
    
    
    int num_pyramid_levels, window_size;
    int num_iterations_kl_tracker;
    float min_displacement_exit_criterion_kl_tracker;
    int source_image_width, source_image_height;
    int total_number_points_being_tracked;
};

#endif