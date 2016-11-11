// CreateBackgrounds.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CmdOptions.h"


int main(int argc, char **argv)
{
	CmdOptions opt(argc, argv);
	if (opt.good) {
		printf("bgdir: %s\n", opt.bgdirname.c_str());
		printf("outdir: %s\n", opt.outdirname.c_str());
		printf("width: %i\n", opt.width);
		printf("height: %i\n", opt.height);
		printf("num: %i\n", opt.num);
		return 0;

	}
	else {
		return 1;
	}
}

