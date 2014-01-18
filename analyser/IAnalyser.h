#pragma once

#include <map>
#include <string>
#include <iostream>

#include "../commons/factory/Factory.h"
#include "../dataModel/IDataModel.h"

using namespace std;

class IAnalyser: public FactoryMethod {

public:

	virtual ~IAnalyser(){}

	virtual void* createType(string &typeId) = 0;

	virtual IDataModel::type getType() = 0;
		
	virtual IDataModel* getFeatures(string name) = 0;
};


