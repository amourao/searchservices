#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>

#include "../commons/factory/Factory.h"
#include "../dataModel/IDataModel.h"

using namespace std;

class IIndexer: public FactoryMethod {

public:

	virtual ~IIndexer(){}
	
	virtual void* createType(string &typeId) = 0;
		
	virtual vector<float> knnSearchId(string name, int n) = 0;

	virtual vector<string> knnSearchName(string name, int n) = 0;

	virtual vector<float> radiusSearchId(string name, double radius) = 0;

	virtual vector<string> radiusSearchName(string name, double radius) = 0;
};


