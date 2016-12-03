#include <opencv2/opencv.hpp>
#include "Misc/Video.h"
#include <iostream>
#include <time.h>

int main(int argc, char** argv)
{
#if 0
	cv::Mat img(RobotVideo::CAPTURE_ROWS, RobotVideo::CAPTURE_COLS, CV_8UC3, cv::Scalar(0));
	cv::putText(img,
		"Hello World on Jetson!",
		cv::Point(10, img.rows / 2),
		cv::FONT_HERSHEY_DUPLEX,
		1.0,
		CV_RGB(118, 185, 0),
		2);
#endif

	if(argc < 2) {
		std::cerr << "Image file name?" << std::endl;
		return 1;
	}

	cv::Mat img = cv::imread(argv[1]);
	cv::Size dispSize(424, 240);
	cv::Mat display;

	RobotVideo* processor = RobotVideo::GetInstance();
	std::ostringstream capturePipe;
	capturePipe << "nvcamerasrc ! video/x-raw(memory:NVMM)"
		<< ", width=(int)" << RobotVideo::CAPTURE_COLS
		<< ", height=(int)" << RobotVideo::CAPTURE_ROWS
		<< ", format=(string)I420, framerate=(fraction)30/1 ! nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
	cv::VideoCapture capture(capturePipe.str());

//gst-launch-1.0 nvcamerasrc fpsRange="30.0 30.0" ! 'video/x-raw(memory:NVMM), width=(int)1920, height=(int)1080, format=(string)I420, framerate=(fraction)30/1' ! nvtee ! nvvidconv flip-method=2 ! 'video/x-raw(memory:NVMM), format=(string)I420' ! nvoverlaysink -e
//cv::VideoCapture capture("nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)1280, height=(int)720,format=(string)I420, framerate=(fraction)30/1 ! nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink");

#if 0
	//open the video stream and make sure it's opened
	//We specify desired frame size and fps in constructor
	//Camera must be able to support specified framesize and frames per second
	//or this will set camera to defaults
	int count=1;
	//while (!capture.open(CAPTURE_PORT, CAPTURE_COLS, CAPTURE_ROWS, CAPTURE_FPS))
	//while (!capture.open(0))

	{
		std::cerr << "Error connecting to camera stream, retrying " << count<< std::endl;
		if(count++ > 3) {
			std::cerr << "Couldn't connect to camera" << std::endl;
			return 1;
		}
		usleep(5 * 1000000);
	}
	capture.set(CV_CAP_PROP_FRAME_WIDTH, RobotVideo::CAPTURE_COLS);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, RobotVideo::CAPTURE_ROWS);

	//After Opening Camera we need to configure the returned image setting
	//all opencv v4l2 camera controls scale from 0.0 to 1.0
	//capture.set(CV_CAP_PROP_EXPOSURE_ABSOLUTE, 0);
	//capture.set(CV_CAP_PROP_BRIGHTNESS, 0);
	//capture.set(CV_CAP_PROP_CONTRAST, 0);
#else
	if(!capture.isOpened()) {
		std::cerr << "Couldn't connect to camera" << std::endl;
		return 1;
	}

#endif

	while(true) {

		capture >> img;
		if (img.empty()) {
			std::cerr << " Error reading from camera, empty frame." << std::endl;
			usleep(5 * 1000000);
			continue;
		}


		double t_start = double(clock())/CLOCKS_PER_SEC;
		processor->ProcessContours(img);

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
		cv::putText(img, oss1.str(), cv::Point(20,RobotVideo::CAPTURE_ROWS-32), 1, 1, ossColor, 1);
		cv::putText(img, oss2.str(), cv::Point(20,RobotVideo::CAPTURE_ROWS-16), 1, 1, ossColor, 1);

		std::ostringstream oss;
		double t_end = double(clock())/CLOCKS_PER_SEC - t_start;
		oss << 1000.0*(t_end) << " ms, " << 1.0 / t_end << " fps";
		cv::putText(img, oss.str(), cv::Point(20,130), 1, 6, cv::Scalar(0, 200,255), 2);

		cv::resize(img, display, dispSize);
		cv::imshow("Hello!", display);
		int key = 0xff & cv::waitKey(5);
		if ((key & 255) == 27) break;
		//if ((key & 255) == 32 && p_file != files.end()) filename = *p_file++;
	}
	return 0;
}

