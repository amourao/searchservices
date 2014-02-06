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

/*
map<string, IDataModel::type> FactoryAnalyser::listTypes() {

	map<string, IDataModel::type> ret;
    map<string, FactoryMethod*>::iterator it;

    for (it = objectTypes.begin(); it != objectTypes.end(); it++) {
        ret[it->first] = ((IAnalyser*)it->second.first)->getType();
    }

	return ret;
}

*/
