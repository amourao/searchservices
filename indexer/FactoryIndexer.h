#pragma once

#include <map>
#include <string>
#include <iostream>
#include "../commons/factory/Factory.h"
#include "IIndexer.h"

using namespace std;

class FactoryIndexer: public Factory {

private:

    static FactoryIndexer* instance;
    
    static bool destroyed;
    
    FactoryIndexer();
    ~FactoryIndexer();

public:

	static FactoryIndexer* getInstance();
	
	/**
       Method to list types registered in the factory.
     */
    map<string,string> listTypes();

};
