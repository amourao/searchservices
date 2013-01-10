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
bool Factory::registerType(string typeId, FactoryMethod* object) {

    if (objectTypes.count(typeId) != 0)
        return false;

    objectTypes[typeId] = object;

    cout << " Registered new FactoryMethod: " << typeId << endl;

    return true;
}

/**
   Factory method.
 */
void* Factory::createType(string &typeId) {

    if (objectTypes.count(typeId) == 0)
        return NULL;

    FactoryMethod* object = objectTypes[typeId];

    return object->createType(typeId);
}

/**
   Factory method.
 */
void Factory::listTypes() {

    map<string, FactoryMethod*>::iterator it;

    for (it = objectTypes.begin(); it != objectTypes.end(); it++) {
        cout << "    " << it->first << endl;
    }
}

