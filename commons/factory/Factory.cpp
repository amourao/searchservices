#include "Factory.h"

/**
   Constructor.
 */
Factory::Factory() {
}

/**
   Destructor.
 */
Factory::~Factory() {
}

/**
   Register method.
 */
bool Factory::registerType(string typeId, FactoryMethod* object, map<string,string> params) {

    if (objectTypes.count(typeId) != 0)
        return false;

    objectTypes[typeId] = make_pair(object,params);
    //cout << " Registered new FactoryMethod: " << typeId << endl;


    return true;
}

/**
   Factory method.
 */
void* Factory::createType(string &typeId) {

    if (objectTypes.count(typeId) == 0)
        return NULL;

    FactoryMethod* object = objectTypes[typeId].first;
    map<string,string> params = objectTypes[typeId].second;

    return object->createType(typeId,params);
}

/**
   Factory method.
 */
void Factory::listTypes() {

    map<string, pair<FactoryMethod*,map<string,string> > >::iterator it;

    for (it = objectTypes.begin(); it != objectTypes.end(); it++) {
        cout << "    " << it->first << endl;
    }
}

