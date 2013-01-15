#pragma once

#include <string>
#include <vector>
#include <utility>
#include <iostream>

using namespace std;

class Extractor
{
public:
	
	virtual ~Extractor()  {}

	virtual void extractFeatures(string filename, vector<pair<vector<float>,vector<float> > >& features){
		cerr << "Not implemented" << endl;
	}
	
	virtual void extractFeatures(string filename, vector<float>& features){
		cerr << "Not implemented" << endl;
	}
	
	virtual void extractFeatures(string filename, vector<pair<string,float> >& features){
		cerr << "Not implemented" << endl;
	}
	
	virtual void extractFeatures(string filename, vector<pair<string,vector<float> > >& features){
		cerr << "Not implemented" << endl;
	}
	
	virtual string getName() {return "";}
};

