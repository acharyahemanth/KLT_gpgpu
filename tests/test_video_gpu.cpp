#include "misc_headers.h"
#include <iostream>
#include "klt_gpu.h"
#include <opencv2/features2d.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>
#include <sys/time.h>

#define num_pyramid_levels 3
#define search_window_size 7

long long unsigned currentTimeInMilliseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}


int main(){
    cv::VideoCapture  source_video = cv::VideoCapture(std::string(BASE_TEST_DIR) + "/tests/test_video.mp4");
    if (!source_video.isOpened()) {
        std::cout << "cannot open video " << std::endl;
        exit(-1);
    }
    
    
    //resize first frame to smaller size, and convert to gray image---
    cv::Mat source_color;
    source_video >> source_color;    
    cv::Mat smallimg=cv::Mat::zeros(360,640,source_color.type());
    cv::resize(source_color, smallimg, smallimg.size());
    cv::Mat source_gray;
    cv::cvtColor(smallimg, source_gray, CV_BGR2GRAY);
    
    
    //Get input corners
    std::vector<cv::Point2f>input_corners;//(kps.size());
    cv::goodFeaturesToTrack(source_gray, input_corners, 100, 0.01, 10);
    
    //Create window and setup GL context---
    int window_width = 640;
    int window_height = 480;
    GLFWwindow* gl_window = setupGL(window_width,window_height,true);
    char *gl_context_version = (char*)glGetString(GL_VERSION);
    std::cout << "GL context version -> " << gl_context_version << std::endl;

    
    
    //Create KLT tracker component--------------------------------
    KLT_gpu klt(num_pyramid_levels,search_window_size,source_gray.cols, source_gray.rows);


    //Go through video, feed all frames through tracker----
    std::vector<cv::Point2f>tracked_corners;
    std::vector<bool>error;
    cv::Mat query_frame, query_gray, query_small;
    cv::Mat prev_image = source_gray.clone();  
    int frame_ctr=0;
    do{
        source_video >> query_frame;
        if(query_frame.empty()) {
            break;
        }        
        cv::resize(query_frame, query_small, smallimg.size());
        cv::cvtColor(query_small, query_gray, CV_BGR2GRAY);

        long long unsigned start = currentTimeInMilliseconds();
        klt.execute(prev_image, query_gray, input_corners, tracked_corners, error);
        std::cout << "Exec time : " << (int)(currentTimeInMilliseconds() - start) << std::endl;

        input_corners.clear();
        for(int i=0;i<tracked_corners.size();i++){
            if(!error[i]){
                input_corners.push_back(tracked_corners[i]);
                cv::circle(query_small, tracked_corners[i], 2, cv::Scalar(0,255,0),cv::FILLED);
            }
        }
        std::cout << "Frame #" << frame_ctr << " tracked " << input_corners.size() << " corners" << std::endl;

        prev_image = query_gray.clone();
        frame_ctr++;
        cv::imshow("tracked corners", query_small);
        cv::waitKey(1);
    }while(1);

}

