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
<<<<<<< HEAD
	string getQuery(int index);
=======
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d

private:

	static AppConfig* instance;
	map<string,vector<pair<string,string> > > metadata;
	map<string,vector<map<string,string> > > featuresRoi;
	map<string,vector<map<string,string> > > featuresText;
	map<string,vector<map<string,string> > > featuresVec;
<<<<<<< HEAD
	vector<string> queries;
=======
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d

	AppConfig();

	void parseMetadata(Value appMetadata);
	map<string,vector<map<string,string> > > parseFeatures(Value feature);
<<<<<<< HEAD
	void parseQuerys(Value querys);

=======
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d

};
