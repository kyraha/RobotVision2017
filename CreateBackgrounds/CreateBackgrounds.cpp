// CreateBackgrounds.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CmdOptions.h"
#include <iostream>
#include <fstream>

class MyApp {
public:
	bool good;
	std::string bgdirname, outdirname;
	int num, width, height;
	MyApp(int argc, char **argv);
	bool parse(int argc, char **argv);
	std::string help();
	bool readAll();
	bool processOne(const std::string &in);
};

MyApp::MyApp(int argc, char ** argv)
	: good(false)
	, bgdirname("./")
	, outdirname("./")
	, num(0)
	, width(0)
	, height(0)
{
	good = parse(argc, argv);
}

bool MyApp::parse(int argc, char **argv)
{
	CmdOptions input(argc, argv);
	try {
		if (input.has("-bg")) bgdirname = input.get("-bg");
		if (input.has("-outdir")) outdirname = input.get("-outdir");
		if (input.has("-num")) num = stoi(input.get("-num"));
		if (input.has("-w")) width = stoi(input.get("-w"));
		if (input.has("-h")) height = stoi(input.get("-h"));
	}
	catch (const std::exception& e) {
		std::cerr << "Error in the input parameters: " << e.what() << std::endl;
		return false;
	}
	return true;
}

bool MyApp::processOne(const std::string &in)
{
	cv::Mat whole;
	whole = cv::imread(in);
	if (whole.empty()) return false;
	std::cout << " image " << whole.cols << "x" << whole.rows;
	return true;
}

bool MyApp::readAll()
{
	printf("bgdir: %s\n", bgdirname.c_str());
	printf("outdir: %s\n", outdirname.c_str());
	printf("width: %i\n", width);
	printf("height: %i\n", height);
	printf("num: %i\n", num);
	std::ifstream filelist(bgdirname);
	std::string filename;
	while (filelist.good()) {
		std::getline(filelist, filename);
		if (filename.empty()) continue;
		std::cout << "Processing file: " << filename;
		std::cout << (processOne(filename) ? " OK" : " FAILED") << std::endl;
	}
	if(good) return true;
	else return false;
}

int main(int argc, char **argv)
{
	MyApp app(argc, argv);
	if (app.good) {
		return app.readAll();
	}
	else return false;
}

