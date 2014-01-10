#include "FactoryIndexer.h"

FactoryIndexer* FactoryIndexer::instance = NULL;

bool FactoryIndexer::destroyed = true;

FactoryIndexer::FactoryIndexer(){
	destroyed = false;
}

FactoryIndexer::~FactoryIndexer(){
	destroyed = true;
}
 
FactoryIndexer* FactoryIndexer::getInstance(){
	if(destroyed)
		instance = new FactoryIndexer();
	
	if (instance == NULL)
		instance = new FactoryIndexer();
	
	return instance;
}

map<string,string> FactoryIndexer::listTypes() {

	map<string, string> ret;
    map<string, FactoryMethod*>::iterator it;

    for (it = objectTypes.begin(); it != objectTypes.end(); it++) {
        ret[it->first] = it->first;
    }

	return ret;
}

