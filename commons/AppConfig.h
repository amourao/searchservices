#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <fstream>

using namespace std;
using namespace Json;

class AppConfig
{

public:

	static AppConfig* getInstance();

	~AppConfig();

	map<string,vector<pair<string,string> > > getMetadata();

private:

	static AppConfig* instance;
	map<string,vector<pair<string,string> > > metadata;

	AppConfig();

	void parseMetadata(Value appMetadata);

};
