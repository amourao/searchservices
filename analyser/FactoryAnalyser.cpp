#include "FactoryAnalyser.h"

FactoryAnalyser* FactoryAnalyser::instance = NULL;

bool FactoryAnalyser::destroyed = true;

FactoryAnalyser::FactoryAnalyser(){
	destroyed = false;
}

FactoryAnalyser::~FactoryAnalyser(){
	destroyed = true;
}
 
FactoryAnalyser* FactoryAnalyser::getInstance(){
	if(destroyed)
		instance = new FactoryAnalyser();
	
	if (instance == NULL)
		instance = new FactoryAnalyser();
	
	return instance;
}
