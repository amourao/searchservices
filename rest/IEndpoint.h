#pragma once

#include <string>
<<<<<<< HEAD
<<<<<<< HEAD
#include <iostream>
=======
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d
=======
#include <iostream>
>>>>>>> 3e13f64ef9b5722f6ed80d2b5d5889fb96c65852

#include <Poco/Net/HTTPServerResponse.h>

#include "../commons/factory/Factory.h"

using namespace Poco::Net;
using namespace std;

class IEndpoint: public FactoryMethod {

public:

	virtual ~IEndpoint(){}
	
<<<<<<< HEAD
<<<<<<< HEAD
	virtual void* createType(string& typeId) = 0;
	
	virtual void handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp) = 0;
=======
	virtual void* createType(string &typeId) = 0;
	
	virtual void handleRequest(string method, map<string, string> queryStrings, HTTPServerResponse &resp) = 0;
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d
=======
	virtual void* createType(string& typeId) = 0;
	
	virtual void handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp) = 0;
>>>>>>> 3e13f64ef9b5722f6ed80d2b5d5889fb96c65852

};
