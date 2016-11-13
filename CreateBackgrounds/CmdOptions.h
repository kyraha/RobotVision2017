#pragma once

#include "stdafx.h"
#include <opencv2/highgui.hpp>



class CmdOptions {
public:
	CmdOptions(int &argc, char **argv);
	const std::string& get(const std::string &option) const;
	bool has(const std::string &option) const;
private:
	std::vector <std::string> tokens;
};

