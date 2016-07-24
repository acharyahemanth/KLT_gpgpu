#ifndef KLT_CPUH
#define KLT_CPUH

#include "misc_headers.h"
#include <vector>


class KLT_cpu{
public:
	KLT_cpu(int num_pyramid_levels, int window_size);
	~KLT_cpu();
	void execute(cv::Mat source,
			     cv::Mat dest,
			     std::vector<cv::Point2f>src_pts,
			     std::vector<cv::Point2f>&tracked_pts,
			     std::vector<bool>error);
private:
	std::vector<cv::Point2f> iterativeTrackerAtAPyramidLevel(cv::Mat source,
										 cv::Mat dest,
										 std::vector<cv::Point2f>source_points,
										 std::vector<cv::Point2f>prediction,
										 std::vector<bool>&error,
										 int pyramid_level);
    void getAb(cv::Point2f p, cv::Point2f pred, cv::Mat src, cv::Mat dest, int pyramid_level, cv::Mat &A, cv::Mat &b);
    bool isPointWithinImage(cv::Point2f current_p,int pyramid_level);
    cv::Point2f getGradient(cv::Point2f current_p,cv::Mat src, int pyramid_level);
    int getPel(cv::Mat img, float x, float y, int pyramid_level);
    int blt(cv::Mat img, float x, float y);
    
	int num_pyramid_levels;
	int window_size;
    int source_image_width, source_image_height;
    int num_iterations_kl_tracker;
    float min_displacement_exit_criterion_kl_tracker;
};

#endif