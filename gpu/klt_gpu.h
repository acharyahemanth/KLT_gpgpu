#ifndef KLTGPUH
#define KLTGPUH

#include "gl_apis.h"
#include "../common/mylog.h"
#include <opencv2/opencv.hpp>


class KLT_gpu{
public:
    KLT_gpu(int num_pyramid_levels, int window_size, int image_width, int image_height);
    ~KLT_gpu();
    void execute(cv::Mat source,
                 cv::Mat dest,
                 std::vector<cv::Point2f>src_pts,
                 std::vector<cv::Point2f>&tracked_pts,
                 std::vector<bool>&error);
    void execute_dbg();
    void drawFrame(cv::Mat img, int screen_width, int screen_height, std::vector<cv::Point2f>tracked_corners, std::vector<bool>error);
    void execute_ocv(cv::Mat source,
                     cv::Mat dest,
                     std::vector<cv::Point2f>src_pts,
                     std::vector<cv::Point2f>&tracked_pts,
                     std::vector<bool>&error);
    bool isNextIterationReqd();

private:
    void iterativeTrackerAtAPyramidLevel(int pyramid_level);
    void loadTexturesWithData(cv::Mat source,
                              cv::Mat dest,
                              std::vector<cv::Point2f>source_points,
                              std::vector<cv::Point2f>prediction);
    void calcA(int pyramid_level);
    void calcb(int pyramid_level);
    void track(int pyramid_level);
    void projectPointsToNextLevel();
    void populateOutputDS(std::vector<cv::Point2f>&tracked_pts, std::vector<bool>&error);
    void setupTextures();
    std::string printMatString(cv::Mat);
                              
    //Shader variables
    GLuint source_image_id, dest_image_id, back_image_id;// source_points_id[2];
//    GLuint prediction_points_id[2];
    //getA
    GLuint getA_shader_id, getA_sh_srcimage_texture_sampler_id, getA_sh_srcpts_texture_sampler_id;
    GLuint getA_sh_num_points_id, getA_sh_window_size_id, getA_sh_image_width_id, getA_sh_image_height_id, getA_sh_vao_id, getA_sh_vert_id, getA_sh_pyramid_level_id;
    //getb
    GLuint getb_shader_id, getb_sh_srcimage_texture_sampler_id, getb_sh_srcpts_texture_sampler_id;
    GLuint getb_sh_num_points_id, getb_sh_window_size_id, getb_sh_image_width_id, getb_sh_image_height_id, getb_sh_vao_id, getb_sh_vert_id, getb_sh_dstimage_texture_sampler_id, getb_sh_predpts_texture_sampler_id, getb_sh_pyramid_level_id;
    //track
    GLuint track_shader_id, track_sh_srcpts_texture_sampler_id;
    GLuint track_sh_num_points_id, track_sh_window_size_id, track_sh_image_width_id, track_sh_image_height_id, track_sh_vao_id, track_sh_vert_id, track_sh_predpts_texture_sampler_id, track_sh_pyramid_level_id, track_sh_A_texture_sampler_id, track_sh_b_texture_sampler_id;
    //next_level
    GLuint next_level_shader_id, next_level_sh_srcpts_texture_sampler_id, next_level_sh_predpts_texture_sampler_id, next_level_sh_vao_id, next_level_sh_num_points_id,
    next_level_sh_vert_id;
    //back_image
    GLuint back_image_shader_id, back_image_sh_vert_id, back_image_sh_srcimage_texture_sampler_id, back_image_sh_image_width_id, back_image_sh_image_height_id, back_image_sh_vao_id;
    
    //dbgsh
    GLuint dbg_sh_id, dbg_sh_vert_id, dbg_sh_vao_id, dbg_sh_ip_texture_sampler;    
    int ppong_idx_iterations, ppong_idx_pyramid_level;
    GLuint fbo_id;

    
    //Output textures
    GPGPUOutputTexture getA_sh_Amat_output;
    GPGPUOutputTexture getb_sh_bmat_output;
    GPGPUOutputTexture track_sh_prediction_output[2];
    GPGPUOutputTexture next_sh_source_points_output[2];
    GPGPUOutputTexture point_shift_delta;
    
    int num_pyramid_levels, window_size;
    int num_iterations_kl_tracker;
    float min_displacement_exit_criterion_kl_tracker;
    int source_image_width, source_image_height;
    int max_number_of_points_supported, total_number_points_being_tracked, margin_to_declare_tracking_lost;
};

#endif