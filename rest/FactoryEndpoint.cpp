#include "FactoryEndpoint.h"

FactoryEndpoint* FactoryEndpoint::instance = NULL;

bool FactoryEndpoint::destroyed = true;

FactoryEndpoint::FactoryEndpoint(){
	destroyed = false;
}

FactoryEndpoint::~FactoryEndpoint(){
	destroyed = true;
}
 
FactoryEndpoint* FactoryEndpoint::getInstance(){
	if(destroyed)
		instance = new FactoryEndpoint();
	
	if (instance == NULL)
		instance = new FactoryEndpoint();
	
	return instance;
}

