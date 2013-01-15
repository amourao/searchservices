#pragma once

#include <map>
#include <string>
#include <iostream>
#include "Factory.h"
#include "extractors/IAnalyser.h"
#include "../dataModel/IDataModel.h"

using namespace std;

class FactoryAnalyser: public Factory {

private:

    static FactoryAnalyser* instance;
    
    static bool destroyed;
    
    FactoryAnalyser();
    ~FactoryAnalyser();

public:
	
	static FactoryAnalyser* getInstance();
	
	/**
       Method to list types registered in the factory.
     */
    map<string, IDataModel::type> listTypes();

};
