#pragma once

#include "stdafx.h"
#include <opencv2/highgui.hpp>

class CmdOptions {
public:
	bool good;
	std::string bgdirname, outdirname;
	int num, width, height;
	CmdOptions(int argc, char **argv);
	bool parse(int argc, char **argv);
	std::string help();
};

