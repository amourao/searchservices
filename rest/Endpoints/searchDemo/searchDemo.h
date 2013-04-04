#pragma once

#include <iostream>
#include <sstream>

#include <Poco/RegularExpression.h>

#include "../../FactoryEndpoint.h"
#include "../../IEndpoint.h"

using namespace std;
using namespace Poco;
using namespace Json;
using namespace flann;


class searchDemo : public IEndpoint{
public:
	searchDemo();
	searchDemo(string type);
	virtual ~searchDemo();

	void* createType(string &typeId);
	void handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp);

private:
	string type;
	flann::Index<flann::L2<float> > indexTags;
	flann::Index<flann::L2<float> > indexVisual;
	flann::Index<flann::L2<float> > indexGenre;

	string searchText(map<string, string> parameters);
	string searchSimilarTag(map<string, string> parameters);
	string searchSimilarVisual(map<string, string> parameters);
	string searchSimilarGenre(map<string, string> parameters);

	void readBinPoints(string fname, Matrix<float> &mat, int nrows);
	void readFileNames(string fname, vector<string>* fileNames);
};

