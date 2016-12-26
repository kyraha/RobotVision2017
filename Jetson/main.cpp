#include <opencv2/opencv.hpp>
#include "Misc/Video.h"
#include <iostream>
#include <time.h>

int main(int argc, char** argv)
{
	const cv::Size dispSize(848, 480);
	int fontScale = 2 * RobotVideo::CAPTURE_ROWS / dispSize.height;

	cv::Mat img;
	cv::Mat display;

	std::ostringstream capturePipe;
	capturePipe << "nvcamerasrc ! video/x-raw(memory:NVMM)"
		<< ", width=(int)" << RobotVideo::CAPTURE_COLS
		<< ", height=(int)" << RobotVideo::CAPTURE_ROWS
		<< ", format=(string)I420, framerate=(fraction)30/1 ! "
		<< "nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! "
		<< "videoconvert ! video/x-raw, format=(string)BGR ! appsink";
	cv::VideoCapture capture(capturePipe.str());

//gst-launch-1.0 nvcamerasrc fpsRange="30.0 30.0" ! 'video/x-raw(memory:NVMM), width=(int)1920, height=(int)1080, format=(string)I420, framerate=(fraction)30/1' ! nvtee ! nvvidconv flip-method=2 ! 'video/x-raw(memory:NVMM), format=(string)I420' ! nvoverlaysink -e
//cv::VideoCapture capture("nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)1280, height=(int)720,format=(string)I420, framerate=(fraction)30/1 ! nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink");

	if(!capture.isOpened()) {
		std::cerr << "Couldn't connect to camera" << std::endl;
		return 1;
	}

	RobotVideo* processor = RobotVideo::GetInstance();
	std::vector<double> total_times;
	double total_start = double(clock())/CLOCKS_PER_SEC;
	for(size_t n = 1;; ++n) {

		capture >> img;
		if (img.empty()) {
			std::cerr << " Error reading from camera, empty frame." << std::endl;
			usleep(5 * 1000000);
			continue;
		}


		double t_start = double(clock())/CLOCKS_PER_SEC;
		std::vector<double> times = processor->ProcessContours(img);
		if(total_times.empty()) total_times = times;
		else {
			total_times[0] = n*n / (double(clock())/CLOCKS_PER_SEC - total_start);
			for(size_t i = 1; i < times.size() && i < total_times.size(); ++i)
				total_times[i] += times[i] - times[0];
		}

		// Draw pink boxes around detected targets
		for (std::vector<cv::Point> box : processor->m_boxes) {
			std::vector<cv::Point> crosshair;
			crosshair.push_back(cv::Point(box[0].x-5, box[0].y-5));
			crosshair.push_back(cv::Point(box[1].x+5, box[1].y-5));
			crosshair.push_back(cv::Point(box[2].x+5, box[2].y+5));
			crosshair.push_back(cv::Point(box[3].x-5, box[3].y+5));
			cv::polylines(img, crosshair, true, cv::Scalar(260, 0, 255),2);
		}

		std::ostringstream oss1, oss2;
		cv::Scalar ossColor(260, 0, 255);
		if (processor->HaveHeading() > 0) {
			oss1 << "Turn: ";
			oss2 << "Dist: ";
			if (processor->HaveHeading() > 1) {
				oss1 << processor->GetTurn(0) << " : " << processor->GetTurn(1);
				oss2 << processor->GetDistance(0) << " : " << processor->GetDistance(1);
			}
			else {
				oss1 << processor->GetTurn();
				oss2 << processor->GetDistance();
			}
		}
		else {
			oss1 << "No target";
			oss2 << "No target";
			ossColor = cv::Scalar(0, 100,255);
		}
		cv::putText(img, oss1.str(), cv::Point(20,RobotVideo::CAPTURE_ROWS-8*fontScale), 1, fontScale, ossColor, 1);
		cv::putText(img, oss2.str(), cv::Point(20,RobotVideo::CAPTURE_ROWS-16*fontScale), 1, fontScale, ossColor, 1);

		std::ostringstream oss;
		double t_end = double(clock())/CLOCKS_PER_SEC - t_start;
		oss << 1000.0*(t_end) << " ms, " << 1.0 / t_end << " fps";
		cv::putText(img, oss.str(), cv::Point(20,16*fontScale), 1, fontScale, cv::Scalar(0, 200,255), 2);

		cv::resize(img, display, dispSize);
		for(int i=0; i < total_times.size(); ++i) {
			std::ostringstream osst;
			osst << i << ": " << total_times[i] / n;
			cv::putText(display, osst.str(), cv::Point(20,160+i*16), 1, 1, cv::Scalar(90,255,90), 1);
		}

		if(n%60 == 0) cv::imshow("Hello!", display);

		int key = 0xff & cv::waitKey(5);
		if ((key & 255) == 27) break;
		if ((key & 255) == 32 ) {
			cv::waitKey();
		}
	}
	return 0;
}

