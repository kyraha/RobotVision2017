#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include <algorithm>
#include <time.h>
#include <iostream>
#include <time.h>
#include <unistd.h>

static const cv::Size frameSize(1280, 720);
static const cv::Size dispSize(640, 360);
static const cv::Scalar fColor(60, 0, 200);

int main(int argc, char** argv)
{
	const cv::HersheyFonts font = cv::FONT_HERSHEY_PLAIN;
	const int fHeight = 12;
	const double fScale = double(dispSize.height) / fHeight / 25;
	const int fLine = fScale * fHeight;
	const int fBold = fScale + 0.5;

	cv::Mat img;
	cv::Mat sec;
	cv::Mat display;
	cv::Mat display2;

	cv::VideoCapture capture;
	cv::VideoCapture secondary;
	std::ostringstream capturePipe;
	capturePipe << "nvcamerasrc ! video/x-raw(memory:NVMM)"
		<< ", width=(int)" << frameSize.width
		<< ", height=(int)" << frameSize.height
		<< ", format=(string)I420, framerate=(fraction)30/1 ! "
		<< "nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! "
		<< "videoconvert ! video/x-raw, format=(string)BGR ! appsink";
	if(!capture.open(capturePipe.str())) {
		capture.open(0);
		capture.set(cv::CAP_PROP_FRAME_WIDTH, frameSize.width);
		capture.set(cv::CAP_PROP_FRAME_HEIGHT, frameSize.height);
		std::cerr << "Resolution: "<< capture.get(cv::CAP_PROP_FRAME_WIDTH)
			<< "x" << capture.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;
	}
	else {
		secondary.open(0);
		secondary.set(cv::CAP_PROP_FRAME_WIDTH, frameSize.width);
		secondary.set(cv::CAP_PROP_FRAME_HEIGHT, frameSize.height);
		std::cerr << "Secondary Resolution: "<< secondary.get(cv::CAP_PROP_FRAME_WIDTH)
			<< "x" << secondary.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;
	}
	if(!capture.isOpened()) {
		std::cerr << "Couldn't connect to primary camera" << std::endl;
		return 1;
	}
	if(!secondary.isOpened()) {
		std::cerr << "Couldn't connect to secondary camera" << std::endl;
		return 1;
	}

	std::vector<double> total_times;
	double total_start = double(clock())/CLOCKS_PER_SEC;
	for(size_t n = 1;; ++n) {

		capture >> img;
		secondary >> sec;
		if (img.empty()) {
			std::cerr << " Error reading from primary camera, empty frame." << std::endl;
			usleep(5 * 1000000);
			continue;
		}
		if (sec.empty()) {
			std::cerr << " Error reading from secondary camera, empty frame." << std::endl;
			usleep(5 * 1000000);
			continue;
		}

		double t_start = double(clock())/CLOCKS_PER_SEC;

		cv::resize(img, display, dispSize);
		cv::resize(sec, display2, dispSize);

		std::ostringstream oss;
		double t_end = double(clock())/CLOCKS_PER_SEC - total_start;
		oss << "Clock: " << 1000.0*(t_end) << " ms, Frame # " << n << " Rate: " << double(n)/t_end << " fps";
		cv::putText(display, oss.str(), cv::Point(20,2*fLine), font, fScale, fColor, fBold);
		cv::putText(display2, oss.str(), cv::Point(20,2*fLine), font, fScale, fColor, fBold);

		cv::imshow("Primary", display);
		cv::imshow("Secondary",display2);
		if(n%30==0) std::cout << oss.str() << std::endl;

		int key = 0xff & cv::waitKey(5);
		if ((key & 255) == 27) break;

	}
	return 0;
}

