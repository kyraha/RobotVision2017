// RobotVision2017.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


int main(int argc, char **argv)
{
	cv::Mat image;
	if (argc > 1) {
		std::string filename = argv[1];
		image = cv::imread(filename);
	}
	else {
		image = cv::Mat(400, 400, CV_8UC3);
		cv::circle(image, cv::Point(200, 200), 100, cv::Scalar(255, 0, 200));
	}
	cv::imshow("Image", image);
	cv::waitKey();
}