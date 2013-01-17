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
	
	void handleRequest(string method, map<string, string> queryStrings, HTTPServerResponse &resp);

private:
  string type;
};

