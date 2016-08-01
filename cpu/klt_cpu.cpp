#include "klt_cpu.h"
#include <assert.h> 
#include <iostream>

KLT_cpu::KLT_cpu(int num_pyramid_levels, int window_size){
	this->num_pyramid_levels = num_pyramid_levels;
	assert(window_size%2!=0);
	this->window_size = window_size;
    
    num_iterations_kl_tracker = 2;//20;
    min_displacement_exit_criterion_kl_tracker = 1e-4;
}

KLT_cpu::~KLT_cpu(){

}

void KLT_cpu::execute(cv::Mat source,
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

    //At topmost level, prediction is just the source points---
    std::vector<cv::Point2f>prediction = src_pts;
    error.clear();
	error.resize(src_pts.size(),false);
    
    //Start from topmost layer and calc optical flow for all pts
	for(int l=num_pyramid_levels-1; l>=0; l--){
        tracked_pts = iterativeTrackerAtAPyramidLevel(source,
										dest,
										src_pts,
                                        prediction,
										error,
										l);

        //project src_pts+tracked_pts to next level
        for(int i=0;i<src_pts.size();i++){
            src_pts[i] = 2*src_pts[i];
            prediction[i] = 2*tracked_pts[i];
        }
	}
}

std::vector<cv::Point2f> KLT_cpu::iterativeTrackerAtAPyramidLevel(cv::Mat source,
                                              cv::Mat dest,
                                              std::vector<cv::Point2f>source_points,
                                              std::vector<cv::Point2f>prediction,
                                              std::vector<bool>&error,
                                              int pyramid_level){
    for(int i=0;i<source_points.size();i++){

        //Form matrix A once for this source point---
        cv::Mat W;//weight matrix
        cv::Mat A;
        getA(source_points[i], source, pyramid_level, A, W);
        cv::Mat A_t = A.t();
//        std::cout << "Point :" << source_points[i] << " A matrix : " << std::endl << A << std::endl;
        
        for(int k=0;k<num_iterations_kl_tracker;k++){
            if(error[i])
                continue;

            //Form matrix b---
            cv::Mat b;
            getb(source_points[i], prediction[i], source, dest, pyramid_level, b, W);
//            std::cout << "b mat -> " << std::endl << b << std::endl;
            
            //Check if the point is trackable looking at eigen values of matrix to be inverted
            
            //Evaluate tracked point
            cv::Mat tracked_pt = (A_t*W*A).inv() * A_t * W * b;
            prediction[i] = prediction[i] + cv::Point2f(tracked_pt);
            if(!isPointWithinImage(prediction[i], pyramid_level))
                error[i] = true;
            std::cout << "iteration " << k << " tracked pt : " << prediction[i] << std::endl;
            
            //Bail out if SAD > threshold : account for moving objects in scene

            //If point is not moving much, move onto next point---
            if(cv::norm(tracked_pt) < min_displacement_exit_criterion_kl_tracker)
                break;
        }
    }
    return prediction;
}

void KLT_cpu::getA(cv::Point2f p, cv::Mat src, int pyramid_level, cv::Mat &A, cv::Mat &W){
    A = cv::Mat::zeros(window_size*window_size, 2, CV_32FC1);
    W = cv::Mat::zeros(window_size*window_size, window_size*window_size, CV_32FC1);
    
    int half_window_size = window_size/2;
    int pel_ctr=0;
    for(int y=-half_window_size;y<=half_window_size;y++){
        for(int x=-half_window_size;x<=half_window_size;x++){
            cv::Point2f p_i = p+cv::Point2f(x,y);
            if(!isPointWithinImage(p_i,pyramid_level))
                continue;
            
            //Set weight to 1 to declare as valid
            W.at<float>(pel_ctr,pel_ctr) = 1;
            
            //Calculate value for A matrix
            cv::Point2f g_xy = getGradient(p_i,src, pyramid_level);
            A.at<float>(pel_ctr,0) = g_xy.x;
            A.at<float>(pel_ctr,1) = g_xy.y;

            pel_ctr++;
        }
    }
}

void KLT_cpu::getb(cv::Point2f p, cv::Point2f pred, cv::Mat src, cv::Mat dest, int pyramid_level, cv::Mat &b, cv::Mat &W){
    b = cv::Mat::zeros(window_size*window_size, 1, CV_32FC1);
    int half_window_size = window_size/2;
    
    int pel_ctr=-1;
    for(int y=-half_window_size;y<=half_window_size;y++){
        for(int x=-half_window_size;x<=half_window_size;x++){
            pel_ctr++;
            if(W.at<float>(pel_ctr,pel_ctr) < 1e-3)
                continue;
                
            cv::Point2f p_i = p+cv::Point2f(x,y);
            cv::Point2f pred_i = pred+cv::Point2f(x,y);
            if(!isPointWithinImage(p_i,pyramid_level) ||
               !isPointWithinImage(pred_i,pyramid_level)){
                   W.at<float>(pel_ctr,pel_ctr) = 0;
                   continue;
            }
            
            //Calculate value for b matrix
            int b_val = getPel(src, p_i.x, p_i.y, pyramid_level) -
                        getPel(dest, pred_i.x, pred_i.y, pyramid_level);
            b.at<float>(pel_ctr,0) = b_val;
        }
    }
}

void KLT_cpu::getAb(cv::Point2f p, cv::Point2f pred, cv::Mat src, cv::Mat dest, int pyramid_level, cv::Mat &A, cv::Mat &b){
    std::vector<cv::Point2f>a_vals;
    std::vector<float>b_vals;
    int half_window_size = window_size/2;
    
    for(int y=-half_window_size;y<=half_window_size;y++){
        for(int x=-half_window_size;x<=half_window_size;x++){
            cv::Point2f p_i = p+cv::Point2f(x,y);
            cv::Point2f pred_i = pred+cv::Point2f(x,y);
            if(!isPointWithinImage(p_i,pyramid_level))
                continue;
            if(!isPointWithinImage(pred_i,pyramid_level))
                continue;
            
            //Calculate value for A matrix
            cv::Point2f g_xy = getGradient(p_i,src, pyramid_level);
            a_vals.push_back(g_xy);
            
            //Calculate value for b matrix
            int b_val = getPel(src, p_i.x, p_i.y, pyramid_level) -
                        getPel(dest, pred_i.x, pred_i.y, pyramid_level);
            b_vals.push_back(b_val);
            
//            std::cout << b_val << std::endl;
//            cv::waitKey();
        }
    }
    
    A = cv::Mat::zeros(a_vals.size(), 2, CV_32FC1);
    b = cv::Mat::zeros(a_vals.size(), 1, CV_32FC1);
    for(int i=0;i<a_vals.size();i++){
        A.at<float>(i,0) = a_vals[i].x;
        A.at<float>(i,1) = a_vals[i].y;
        b.at<float>(i,0) = b_vals[i];
    }
}

bool KLT_cpu::isPointWithinImage(cv::Point2f current_p,int pyramid_level){
    int current_level_width = source_image_width/(1<<pyramid_level);
    int current_level_height = source_image_height/(1<<pyramid_level);
    if(current_p.x >= 0 &&
       current_p.y >= 0 &&
       current_p.x < current_level_width &&
       current_p.y < current_level_height){
        return true;
    }
    else{
        return false;
    }
}

cv::Point2f KLT_cpu::getGradient(cv::Point2f p,cv::Mat src, int pyramid_level){
    const float gx_coeffs[9] = {-1,0,1,-2,0,2,-1,0,1};
    const float gy_coeffs[9] = {-1,-2,-1,0,0,0,1,2,1};
    int gx=0, gy=0;

    int filter_ctr=0;
    for(int y=-1;y<=1;y++){
        for(int x=-1;x<=1;x++){
            int current_pel = getPel(src, p.x+x, p.y+y, pyramid_level);
            gx += gx_coeffs[filter_ctr]*current_pel;
            gy += gy_coeffs[filter_ctr]*current_pel;
            filter_ctr++;
        }
    }
    
    return cv::Point2f(gx, gy);
}

int KLT_cpu::blt(cv::Mat img, float x, float y){
    int lu_x = (int)x;
    int lu_y = (int)y;
    int rd_x = (int)(x+1);
    int rd_y = (int)(y+1);
    float alpha_x = x-lu_x;
    float alpha_y = y-lu_y;
    
    float x_interp_lu = (1-alpha_x)*img.at<uchar>(lu_y,lu_x) + alpha_x*img.at<uchar>(lu_y,rd_x);
    float x_interp_rd = (1-alpha_x)*img.at<uchar>(rd_y,lu_x) + alpha_x*img.at<uchar>(rd_y,rd_x);
    float final_interp_val = (1-alpha_y)*x_interp_lu + alpha_y*x_interp_rd;
    
    return (int)(final_interp_val+0.5);//return the rounded value
}

//Gets the pel (x,y) at pyramid_level specified
//Returns 0 for pel outside the image at that level
int KLT_cpu::getPel(cv::Mat img, float x, float y, int pyramid_level){
    float x_level_0 = x * (1<<pyramid_level);
    float y_level_0 = y * (1<<pyramid_level);
    
    if(x_level_0 < 0 ||
       y_level_0 < 0 ||
       x_level_0 >= source_image_width ||
       y_level_0 >= source_image_height){
        return 0;
    }
    
//    int pel = img.at<uchar>((int)y_level_0, (int)x_level_0);//nn interpolation
    int pel = blt(img, x_level_0, y_level_0);
    return pel;
}
