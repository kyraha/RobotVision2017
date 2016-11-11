#include "stdafx.h"
#include "CmdOptions.h"
#include <iostream>

using namespace std;

CmdOptions::CmdOptions(int argc, char ** argv)
	: good(false)
	, bgdirname("./")
	, outdirname("./")
	, num(0)
	, width(0)
	, height(0)
{
	good = parse(argc, argv);
}

bool CmdOptions::parse(int argc, char **argv)
{
	for (int i = 1; i < argc; ++i)
	{
		if (0 == strcmp(argv[i], "-bgdir"))
		{
			if (++i < argc) bgdirname = argv[i];
			else {
				cerr << "Option -bgdir requires a dir name" << endl;
				return false;
			}
		}
		else if (!strcmp(argv[i], "-num"))
		{
			if (++i < argc) num = atoi(argv[i]);
			else {
				cerr << "Option -num requires an int" << endl;
				return false;
			}
		}
		else if (!strcmp(argv[i], "-w"))
		{
			if(++i < argc) width = atoi(argv[i]);
			else {
				cerr << "Option -w requires an int" << endl;
				return false;
			}
		}
		else if (!strcmp(argv[i], "-h"))
		{
			if(++i < argc) height = atoi(argv[i]);
			else {
				cerr << "Option -h requires an int" << endl;
				return false;
			}
		}
		else if (!strcmp(argv[i], "-outdir"))
		{
			if(++i < argc) outdirname = argv[i];
			else {
				cerr << "Option -outdir requires a dir name" << endl;
				return false;
			}
		}
	}
	return true;
}
