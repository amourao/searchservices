#pragma once

#include <string>
<<<<<<< HEAD
#include <iostream>
=======
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d

#include <Poco/Net/HTTPServerResponse.h>

#include "../commons/factory/Factory.h"

using namespace Poco::Net;
using namespace std;

class IEndpoint: public FactoryMethod {

public:

	virtual ~IEndpoint(){}
	
<<<<<<< HEAD
	virtual void* createType(string& typeId) = 0;
	
	virtual void handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp) = 0;
=======
	virtual void* createType(string &typeId) = 0;
	
	virtual void handleRequest(string method, map<string, string> queryStrings, HTTPServerResponse &resp) = 0;
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d

};
