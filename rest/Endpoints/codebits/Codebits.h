#pragma once

#include <iostream>
#include <sstream>

#include <Poco/RegularExpression.h>

#include "finder.h"
#include "ksvd_finder.h"
#include "ksvd_index.h"
#include "omp.h"
#include "../../FactoryEndpoint.h"
#include "../../IEndpoint.h"
//#include "../../../appData/GameImage.h"

using namespace std;
using namespace Poco;
using namespace bits;

class Codebits : public IEndpoint
{

public:
	Codebits();
	Codebits(string type);
	~Codebits();
	
	void* createType(string &typeId);
	
	void handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp);

private:
  string type;
  
  void index(istream& in);
  //TODO: change to vector<GameImage>
  string search(map<string, string> parameters);
  string latests(map<string, string> parameters);
  string best(map<string, string> parameters);
  string scoreboard(map<string, string> parameters);
  string mybestsmile(map<string, string> parameters);
};

