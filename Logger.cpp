#ifndef LOGGER

#define LOGGER

#define FF_ERROR_MESSAGE 0
#define FF_WARNING_MESSAGE 1
#define FF_MESSAGE 2

#include <string>
#include <iostream>

using namespace std;

void logFF(int type, string message){
	if (type == FF_ERROR_MESSAGE)
		cout << "ERROR: ";
	else if (type == FF_WARNING_MESSAGE)
		cout << "WARNING: ";
	else
		cout << "LOG: ";

	cout << message << endl;
}

#endif