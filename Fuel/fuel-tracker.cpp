#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/cudaarithm.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudafilters.hpp"

#include <algorithm>
#include <time.h>
#include <iostream>
#include <time.h>
#include <unistd.h>

/**
 * \brief Color filter numbers
 *
 * We use a green ring-light and its color has Hue about 70 in OpenCV's range (0-180)
 * We care about anything that falls between 65 and 90 by the hue but the saturation and
 * brightness can be in a quite wide range.
 */
static cv::Vec3i BlobLower(26, 63,  60);
static cv::Vec3i BlobUpper(41, 255, 255);
static int cannyThreshold = 160;
static int votesThreshold = 16;
static int minD = 50;
static int minR = 2;
static int maxR = 50;
static int dP = 0;

static const cv::Size frameSize(1280, 720);
static const cv::Size dispSize(848, 480);
static const cv::Size tunaSize(424, 240);

static std::vector<double> times;
/* Microsoft HD3000 camera, 424x240, inches */
/* %YAML:1.0
calibration_time: "02/04/16 16:27:55"
image_width: 424
image_height: 240
board_width: 9
board_height: 6
square_size: 9.3000000715255737e-001
# flags: +zero_tangent_dist
flags: 8
avg_reprojection_error: 2.5414490641601828e-001

static const cv::Matx33d camera_matrix( //: !!opencv-matrix
		3.5268678648676683e+002, 0., 2.0327612059375753e+002, 0.,
		3.5189453914759548e+002, 1.2064326427596691e+002, 0., 0., 1. );
static const cv::Matx<double, 5, 1> distortion_coefficients( //: !!opencv-matrix
		1.1831895351666220e-001, -9.2348154326823140e-001, 0., 0.,
		1.4944420484345493e+000 );
*/

std::vector<cv::Vec3f> DetectFuel(cv::Mat Im) {
	times.push_back(double(clock())/CLOCKS_PER_SEC);
	cv::cuda::GpuMat src, hsvIm;
	cv::cuda::GpuMat channels[3];
	cv::Mat bw;

	// Upload the source data to the GPU and process it there
	src.upload(Im);
	times.push_back(double(clock())/CLOCKS_PER_SEC);

	cv::cuda::cvtColor(src, hsvIm, CV_BGR2HSV);
	times.push_back(double(clock())/CLOCKS_PER_SEC);
	cv::cuda::split(hsvIm, channels);
	times.push_back(double(clock())/CLOCKS_PER_SEC);
	//threshold, reset to zero everything that is above the upper limit
	cv::cuda::threshold(channels[0], channels[0], BlobUpper[0], 255, cv::THRESH_TOZERO_INV);
	cv::cuda::threshold(channels[1], channels[1], BlobUpper[1], 255, cv::THRESH_TOZERO_INV);
	cv::cuda::threshold(channels[2], channels[2], BlobUpper[2], 255, cv::THRESH_TOZERO_INV);
	times.push_back(double(clock())/CLOCKS_PER_SEC);
	//threshold, reset to zero what is below the lower limit, otherwise to 255
	cv::cuda::threshold(channels[0], channels[0], BlobLower[0], 255, cv::THRESH_BINARY);
	cv::cuda::threshold(channels[1], channels[1], BlobLower[1], 255, cv::THRESH_BINARY);
	cv::cuda::threshold(channels[2], channels[2], BlobLower[2], 255, cv::THRESH_BINARY);
	times.push_back(double(clock())/CLOCKS_PER_SEC);
	//combine all three channels and collapse them into one B/W image (to channels[0])
	cv::cuda::bitwise_and(channels[0], channels[1], channels[0]);
	cv::cuda::bitwise_and(channels[0], channels[2], channels[0]);
	times.push_back(double(clock())/CLOCKS_PER_SEC);

	//cv::Ptr<cv::cuda::Filter> filter = cv::cuda::createGaussianFilter(channels[0].type(), channels[1].type(), cv::Size(17,17), 7);
	//filter->apply(channels[0], channels[1]);
	cv::Mat element = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(7,7), cv::Point(3,3));
	cv::Ptr<cv::cuda::Filter> erode = cv::cuda::createMorphologyFilter(cv::MORPH_OPEN, channels[0].type(), element);
	cv::Ptr<cv::cuda::Filter> dilate = cv::cuda::createMorphologyFilter(cv::MORPH_CLOSE, channels[1].type(), element);
	erode->apply(channels[0], channels[1]);
	dilate->apply(channels[1], channels[0]);

	// Return back to the CPU
	channels[0].download(bw);
	times.push_back(double(clock())/CLOCKS_PER_SEC);

	cv::cuda::GpuMat g_circles;
	cv::Ptr<cv::cuda::HoughCirclesDetector> houghCircles = cv::cuda::createHoughCirclesDetector(
		1.0+0.1*dP,	// dp, 
		0.01 * minD * frameSize.height,	// minDist, 
		cannyThreshold+1,	// cannyThreshold, 
		votesThreshold+1,	// votesThreshold, 
		0.01 * minR * frameSize.width,	// minRadius, 
		0.01 * maxR * frameSize.height	// maxRadius
	);
	houghCircles->detect(channels[0], g_circles);

	//cv::Mat circles;
	std::vector<cv::Vec3f> circles;
	circles.resize(g_circles.cols);
	cv::Mat tmpMat(1, g_circles.cols, CV_32FC3, &circles[0]);
	g_circles.download(tmpMat);

	cv::Mat tuna;
	cv::resize(bw, tuna, tunaSize);
	cv::imshow("Object Detection", tuna);

#if 0
	// Convert and filter the image to extract only green pixels
	cv::cvtColor(Im, hsvIm, CV_BGR2HSV);
	times.push_back(double(clock())/CLOCKS_PER_SEC);
	cv::inRange(hsvIm, BlobLower, BlobUpper, BlobIm);
	times.push_back(double(clock())/CLOCKS_PER_SEC);
	BlobIm.convertTo(bw, CV_8UC1);
	times.push_back(double(clock())/CLOCKS_PER_SEC);
#endif

	// Extract Contours. Thanks OpenCV for all the math
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(bw, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	times.push_back(double(clock())/CLOCKS_PER_SEC);

	times.push_back(double(clock())/CLOCKS_PER_SEC);
	return circles;
}


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

	cv::VideoCapture capture;
	cv::VideoCapture secondary;
//gst-launch-1.0 nvcamerasrc fpsRange="30.0 30.0" ! 'video/x-raw(memory:NVMM), width=(int)1920, height=(int)1080, format=(string)I420, framerate=(fraction)30/1' ! nvtee ! nvvidconv flip-method=2 ! 'video/x-raw(memory:NVMM), format=(string)I420' ! nvoverlaysink -e
//cv::VideoCapture capture("nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)1280, height=(int)720,format=(string)I420, framerate=(fraction)30/1 ! nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink");
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
		secondary.set(cv::CAP_PROP_FRAME_WIDTH, 640);
		secondary.set(cv::CAP_PROP_FRAME_HEIGHT, 360);
		std::cerr << "Secondary Resolution: "<< secondary.get(cv::CAP_PROP_FRAME_WIDTH)
			<< "x" << secondary.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;
	}
	if(!capture.isOpened()) {
		std::cerr << "Couldn't connect to camera" << std::endl;
		return 1;
	}

	cv::namedWindow("Object Detection", cv::WINDOW_NORMAL);
	cv::namedWindow("Original", cv::WINDOW_NORMAL);
	//-- Trackbars to set thresholds for RGB values
	cv::createTrackbar("Low H","Object Detection", &BlobLower[0], 255);
	cv::createTrackbar("High H","Object Detection", &BlobUpper[0], 255);
	cv::createTrackbar("Low S","Object Detection", &BlobLower[1], 255);
	cv::createTrackbar("High S","Object Detection", &BlobUpper[1], 255);
	cv::createTrackbar("Low V","Object Detection", &BlobLower[2], 255);
	cv::createTrackbar("High V","Object Detection", &BlobUpper[2], 255);

	cv::createTrackbar("Canny","Original", &cannyThreshold, 255);
	cv::createTrackbar("Votes","Original", &votesThreshold, 255);
//	cv::createTrackbar("minDist","Original", &minD, 100);
//	cv::createTrackbar("minRad","Original", &minR, 100);
//	cv::createTrackbar("maxRad","Original", &maxR, 100);
	cv::createTrackbar("dP","Original", &dP, 100);
	std::vector<double> total_times;
	double total_start = double(clock())/CLOCKS_PER_SEC;
	for(size_t n = 1;; ++n) {

		capture >> img;
		if(secondary.isOpened()) secondary >> sec;
		if (img.empty()) {
			std::cerr << " Error reading from camera, empty frame." << std::endl;
			usleep(5 * 1000000);
			continue;
		}


		double t_start = double(clock())/CLOCKS_PER_SEC;
		std::vector<cv::Vec3f> balls = DetectFuel(img);
		if(total_times.empty()) total_times = times;
		else {
			total_times[0] = n*n / (double(clock())/CLOCKS_PER_SEC - total_start);
			for(size_t i = 1; i < times.size() && i < total_times.size(); ++i)
				total_times[i] += times[i] - times[0];
		}
		times.clear();

#if 0
		// Draw pink boxes around detected targets
		for (std::vector<cv::Point> box : processor->m_boxes) {
			std::vector<cv::Point> crosshair;
			crosshair.push_back(cv::Point(box[0].x-5, box[0].y-5));
			crosshair.push_back(cv::Point(box[1].x+5, box[1].y-5));
			crosshair.push_back(cv::Point(box[2].x+5, box[2].y+5));
			crosshair.push_back(cv::Point(box[3].x-5, box[3].y+5));
			cv::polylines(img, crosshair, true, cv::Scalar(260, 0, 255),2);
		}
#endif

		if(n%1 == 0) {
			cv::resize(img, display, dispSize);

			for(size_t i=0; i < total_times.size(); ++i) {
				std::ostringstream osst;
				osst << i << ": " << total_times[i] / n;
				cv::putText(display, osst.str(), cv::Point(20,(4+i)*fLine), font, fScale, cv::Scalar(90,255,90), fBold);
			}

			std::ostringstream oss;
			double t_end = double(clock())/CLOCKS_PER_SEC - t_start;
			oss << 1000.0*(t_end) << " ms, frame: " << n;
			cv::putText(display, oss.str(), cv::Point(20,2*fLine), font, fScale, cv::Scalar(0, 200,255), fBold);

			double kX = double(dispSize.width) / frameSize.width;
			double kY = double(dispSize.height) / frameSize.height;
			std::ostringstream ossb;
			double x = 0;
			if(balls.size()>0) x = balls.size();
			ossb << "Balls: " << x << " Scale: " << kX << "/" << kY;
			cv::putText(display, ossb.str(), cv::Point(20,20*fLine), font, fScale, cv::Scalar(0, 200,200), fBold);

			for(size_t i=0; i < balls.size(); ++i) {
				cv::Vec3f c = balls[i];
				circle(display, cv::Point(c[0]*kX, c[1]*kY), c[2]*kX, cv::Scalar(0,0,255), 3, cv::LINE_AA);
			}

			cv::imshow("Original", display);
			if(secondary.isOpened()) cv::imshow("Secondary",sec);
		}



		int key = 0xff & cv::waitKey(5);
		if ((key & 255) == 27) break;
		if ((key & 255) == 32 ) {
			cv::waitKey();
		}
		if ((key & 255) == 'w' ) {
			std::ostringstream fname("calib");
			fname << n << ".jpg";
			cv::imwrite(fname.str(), img);
		}

	}
	return 0;
}

