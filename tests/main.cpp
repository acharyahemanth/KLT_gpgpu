#include "misc_headers.h"
#include <iostream>
#include "klt_cpu.h"
#include <opencv2/features2d.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>

#define num_pyramid_levels 3
#define search_window_size 9

int main(){
    cv::Mat src_img = cv::imread(std::string(BASE_TEST_DIR) + "/tests/marker.png");
    if(src_img.empty()){
        std::cout << "Error : Couldnt find input image" << std::endl;
        exit(0);
    }
    
    //resize to smaller size
    cv::Mat smallimg=cv::Mat::zeros(200,200,src_img.type());
    cv::resize(src_img, smallimg, cv::Size(200,200));
    cv::Mat gray;
    cv::cvtColor(smallimg, gray, CV_BGR2GRAY);
    
    //Create tracking variant
    cv::Mat dest_img = 255*cv::Mat::ones(gray.size(), gray.type());
    int shift=5;
    cv::Rect shift_src = cv::Rect(0,0,gray.cols-shift,gray.rows);
    cv::Rect shift_dest = cv::Rect(shift,0,shift_src.width,shift_src.height);
    gray(shift_src).copyTo(dest_img(shift_dest));
    cv::imshow("shifted_img", dest_img);
    
    //Get corners of image---
    std::vector<cv::Point2f>input_corners;//(kps.size());
    cv::goodFeaturesToTrack(gray, input_corners, 10, 0.01, 10);
    cv::Mat ip_copy = gray.clone();
    cv::cvtColor(ip_copy, ip_copy, CV_GRAY2BGR);
    for(int i=0;i<input_corners.size();i++){
        cv::circle(ip_copy, input_corners[i], 2, cv::Scalar(0,255,0), cv::FILLED);
    }
    cv::imshow("Input corners", ip_copy);
//    cv::waitKey();
    
    
    //Run KLT--------------------------------
    KLT_cpu klt(num_pyramid_levels,search_window_size);
    std::vector<cv::Point2f>tracked_corners;
    std::vector<bool>error;
    klt.execute(gray, dest_img, input_corners, tracked_corners, error);
    
    
    //Show output----------------
    cv::Mat output = dest_img.clone();
    cv::cvtColor(output, output, CV_GRAY2BGR);
    std::cout << "Tracked corners are : " << std::endl;
    for(int i=0;i<tracked_corners.size();i++){
        cv::line(output, input_corners[i], tracked_corners[i], cv::Scalar(0,255,0));
        std::cout << input_corners[i] << "->" << tracked_corners[i] << std::endl;
    }
    cv::imshow("Tracked corners lines", output);
    cv::waitKey();

}

//Artifical images---
//int main(){
//    cv::Mat src_gray = cv::Mat::zeros(100,100,CV_8UC1);
//    int x_boundary = src_gray.cols/2;
//    int y_boundary = src_gray.rows/2;
//    cv::Rect upper_quad_roi = cv::Rect(0,0,x_boundary, y_boundary);//fills till boundary-1
//    src_gray(upper_quad_roi).setTo(cv::Scalar(1));
//    cv::Mat dest_gray = cv::Mat::zeros(100,100,CV_8UC1);
//    
//    
//    
//    //Get corners of image---
//    std::vector<cv::Point2f>input_corners(1);
//    input_corners[0] = cv::Point2f(x_boundary-1,y_boundary-1);
//    
//    
//    
//    KLT_cpu klt(num_pyramid_levels,search_window_size);
//    std::vector<cv::Point2f>tracked_corners;
//    std::vector<bool>error;
//    klt.execute(src_gray, dest_gray, input_corners, tracked_corners, error);
//    
//    
//}