#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>

using namespace std;
using namespace Json;

class AppConfig
{

public:

	static AppConfig* getInstance();

	~AppConfig();

	map<string,vector<pair<string,string> > >  getMetadata();
	map<string,vector<map<string,string> > > getFeaturesRoi();
	map<string,vector<map<string,string> > > getFeaturesText();
	map<string,vector<map<string,string> > > getFeaturesVec();

private:

	static AppConfig* instance;
	map<string,vector<pair<string,string> > > metadata;
	map<string,vector<map<string,string> > > featuresRoi;
	map<string,vector<map<string,string> > > featuresText;
	map<string,vector<map<string,string> > > featuresVec;

	AppConfig();

	void parseMetadata(Value appMetadata);
	map<string,vector<map<string,string> > > parseFeatures(Value feature);

};
