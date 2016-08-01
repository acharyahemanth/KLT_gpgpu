#include "misc_headers.h"
#include <iostream>
#include "klt_cpu.h"
#include "klt_gpu.h"
#include <opencv2/features2d.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/opencv.hpp>


#define num_pyramid_levels 3
#define search_window_size 7

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
    std::cout << "Source points are -> " << std::endl;
   for(int i=0;i<input_corners.size();i++){
       cv::circle(ip_copy, input_corners[i], 2, cv::Scalar(0,255,0), cv::FILLED);
       std::cout << input_corners[i] << std::endl;
   }
   cv::imshow("Input corners", ip_copy);
   //    cv::waitKey();
   
    //Create window and setup GL context---
	int window_width = 640;
	int window_height = 480;
	GLFWwindow* gl_window = setupGL(window_width,window_height,true);

	char *gl_context_version = (char*)glGetString(GL_VERSION);
	std::cout << "GL context version -> " << gl_context_version << std::endl;



   //Run KLT--------------------------------
   KLT_gpu klt(num_pyramid_levels,search_window_size);
   std::vector<cv::Point2f>tracked_corners;
   std::vector<bool>error;
   klt.execute(gray, dest_img, input_corners, tracked_corners, error);
//    klt.execute_dbg();
   
    //Run KLT on CPU----------------------
//     std::cout << "CPU output -------------- " << std::endl;
//     KLT_cpu klt_cpu(num_pyramid_levels,search_window_size);
//     tracked_corners.clear();
//     error.clear();
//     klt_cpu.execute(gray, dest_img, input_corners, tracked_corners, error);
    
   
   //Show output----------------
//    cv::Mat output = dest_img.clone();
//    cv::cvtColor(output, output, CV_GRAY2BGR);
//    std::cout << "Tracked corners are : " << std::endl;
//    for(int i=0;i<tracked_corners.size();i++){
//        cv::line(output, input_corners[i], tracked_corners[i], cv::Scalar(0,255,0));
//        std::cout << input_corners[i] << "->" << tracked_corners[i] << std::endl;
//    }
//    cv::imshow("Tracked corners lines", output);
//    cv::waitKey();
   
}

// int main(){
//     //Create window and setup GL context---
// 	int window_width = 640;
// 	int window_height = 480;
// 	GLFWwindow* gl_window = setupGL(window_width,window_height,true);
//
// 	char *gl_context_version = (char*)glGetString(GL_VERSION);
// 	std::cout << "GL context version -> " << gl_context_version << std::endl;
//
//     //create synthetic images---
//     cv::Mat source=cv::Mat::zeros(200,200,CV_8UC1);
//     cv::randu(source, 0, 255);
//     cv::Mat dest = source.clone();
//     cv::randu(dest, 0, 255);
//     //create synthetic corners---
//     std::vector<cv::Point2f>input_corners(10);
//     for(int i=0;i<10;i++){
//         input_corners[i] = cv::Point2f(i+10,i+10);
//     }
//     
//     //Run KLT--------------------------------
//     std::cout << "GPU output -------------- " << std::endl;
//     KLT_gpu klt_gpu(num_pyramid_levels,search_window_size);
//     std::vector<cv::Point2f>tracked_corners;
//     std::vector<bool>error;
//     klt_gpu.execute(source, dest, input_corners, tracked_corners, error);
////     klt.execute_dbg();
//     
//     std::cout << "CPU output -------------- " << std::endl;
//     KLT_cpu klt_cpu(num_pyramid_levels,search_window_size);
//     tracked_corners.clear();
//     error.clear();
//     klt_cpu.execute(source, dest, input_corners, tracked_corners, error);
//
//
// }