#include "FactoryClassifier.h"

FactoryClassifier* FactoryClassifier::instance = NULL;

bool FactoryClassifier::destroyed = true;

FactoryClassifier::FactoryClassifier(){
	destroyed = false;
}

FactoryClassifier::~FactoryClassifier(){
	destroyed = true;
}
 
FactoryClassifier* FactoryClassifier::getInstance(){
	if(destroyed)
		instance = new FactoryClassifier();
	
	if (instance == NULL)
		instance = new FactoryClassifier();
	
	return instance;
}

map<string, IDataModel::type> FactoryClassifier::listTypes() {

	map<string, IDataModel::type> ret;
    map<string, FactoryMethod*>::iterator it;

    for (it = objectTypes.begin(); it != objectTypes.end(); it++) {
        ret[it->first] = IDataModel::NTAG;
    }

	return ret;
}









