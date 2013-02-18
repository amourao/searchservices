#pragma once

#include "iostream"

#include "../FactoryEndpoint.h"
#include "../IEndpoint.h"

using namespace std;

class TestEndpoint : public IEndpoint
{

public:
	TestEndpoint();
	TestEndpoint(string type);
	~TestEndpoint();
	
	void* createType(string &typeId);
	
<<<<<<< HEAD
<<<<<<< HEAD
	void handleRequest(string method, map<string, string> queryStrings, istream& input,  HTTPServerResponse& resp);
=======
	void handleRequest(string method, map<string, string> queryStrings, HTTPServerResponse &resp);
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d
=======
	void handleRequest(string method, map<string, string> queryStrings, istream& input,  HTTPServerResponse& resp);
>>>>>>> 3e13f64ef9b5722f6ed80d2b5d5889fb96c65852

private:
  string type;
};

