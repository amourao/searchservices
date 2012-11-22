#pragma once

#define FF_ERROR_MESSAGE 0
#define FF_WARNING_MESSAGE 1
#define FF_MESSAGE 2

#include <string>
#include <iostream>



using namespace std;

class Logger
{
public:
	Logger();
	~Logger();

	void logFF(int type, string message);

};

