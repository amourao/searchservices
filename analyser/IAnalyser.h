#pragma once

#include <map>
#include <string>
#include <iostream>
#include "Factory.h"

using namespace std;

class IAnalyser: public FactoryMethod {

private:

    

public:
	
	IAnalyser();
	~IAnalyser();
	
	virtual IAnalyser* createType(string &TypeId) = 0;
	
	
};


