#pragma once

#include <string>

#include <Poco/Net/HTTPServerResponse.h>

#include "../commons/factory/Factory.h"

using namespace Poco::Net;
using namespace std;

class IEndpoint: public FactoryMethod {

public:

	virtual ~IEndpoint(){}
	
	virtual void* createType(string &typeId) = 0;
	
	virtual void handleRequest(string method, map<string, string> queryStrings, HTTPServerResponse &resp) = 0;

};
