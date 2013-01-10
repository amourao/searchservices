#pragma once

#include <string>

using namespace std;

class AnalyserDataType {

public:

	AnalyserDataType(void* data, string name);
	~AnalyserDataType();
	
	void* getData();
	string getName();
	
private:
	
	void* data;
	string name;
};
