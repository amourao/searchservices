#pragma once

#include <map>
#include <string>
#include <iostream>
#include "../commons/factory/Factory.h"

using namespace std;

class FactoryAnalyser: public Factory {

private:

    static FactoryAnalyser* instance;
    
    static bool destroyed;
    
    FactoryAnalyser();
    ~FactoryAnalyser();

public:
	
	static FactoryAnalyser* getInstance();
	
	
};
