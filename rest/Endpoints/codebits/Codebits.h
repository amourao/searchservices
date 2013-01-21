#pragma once

#include <iostream>
#include <sstream>

#include <Poco/RegularExpression.h>

#include "../../FactoryEndpoint.h"
#include "../../IEndpoint.h"

using namespace std;
using namespace Poco;

class Codebits : public IEndpoint
{

public:
	Codebits();
	Codebits(string type);
	~Codebits();
	
	void* createType(string &typeId);
	
	void handleRequest(string method, map<string, string> queryStrings, HTTPServerResponse &resp);

private:
  string type;
  
  //TODO: change to vector<GameImage>
  string search(map<string, string> parameters);
  string latests(map<string, string> parameters);
  string best(map<string, string> parameters);
  string scoreboard(map<string, string> parameters);
  string mybestsmile(map<string, string> parameters);
};

