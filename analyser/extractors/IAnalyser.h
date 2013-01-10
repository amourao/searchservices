#pragma once

#include <map>
#include <string>
#include <iostream>

#include "../Factory.h"
#include "AnalyserDataType.h"


using namespace std;

class IAnalyser: public FactoryMethod {

public:

	virtual ~IAnalyser(){}
	
	virtual void* createType(string &typeId) = 0;
	
	virtual AnalyserDataType* getFeatures(string name) = 0;
};


