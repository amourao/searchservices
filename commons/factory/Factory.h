#ifndef FACTORY_H_
#define FACTORY_H_

#include <map>
#include <string>
#include <iostream>
using namespace std;

class FactoryMethod {

public:

    virtual void* createType(string &typeId) = 0;

    virtual void* createType(string &typeId, map<string,string>& params){
        return createType(typeId);
    }

};

class Factory {

protected:

    map<string, pair<FactoryMethod*,map<string,string> > > objectTypes;

public:

    /**
       Constructor.
     */
    Factory();

    /**
       Destructor.
     */
    virtual ~Factory();

    /**
       Factory method.
     */
    void* createType(string &typeId);

    /**
       Method to register models in the factory.
     */
    bool registerType(string typeId, FactoryMethod* object, map<string,string> params = map<string,string>());

    /**
       Method to list types registered in the factory.
     */
    void listTypes();

};

#endif /*FACTORY_H_*/

