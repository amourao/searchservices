#pragma once

#include "../commons/factory/Factory.h"

using namespace std;

class FactoryEndpoint: public Factory {

private:

    static FactoryEndpoint* instance;
    
    static bool destroyed;
    
    FactoryEndpoint();
    ~FactoryEndpoint();

public:
	
	static FactoryEndpoint* getInstance();

};
