#pragma once

#include <iostream>
#include <sstream>
#include "../../FactoryEndpoint.h"
#include "../../IEndpoint.h"

class MedicalImageClassifier: public IEndpoint
{

public:
	MedicalImageClassifier();
	MedicalImageClassifier(std::string type);
	~MedicalImageClassifier();
	
	void* createType(std::string &typeId);
	
	void handleRequest(string method, map<string, string> queryStrings, istream&, HTTPServerResponse &resp);
private:

	std::string type;
};

