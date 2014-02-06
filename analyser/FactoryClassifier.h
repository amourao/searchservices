#pragma once

#include <map>
#include <string>
#include <iostream>
#include "../commons/factory/Factory.h"
#include "IAnalyser.h"
#include "../dataModel/IDataModel.h"

using namespace std;

class FactoryClassifier: public Factory {

private:

    static FactoryClassifier* instance;

    static bool destroyed;

    FactoryClassifier();
    ~FactoryClassifier();

public:

	static FactoryClassifier* getInstance();

	/**
       Method to list types registered in the factory.
     */
    //map<string, IDataModel::type> listTypes();

};
