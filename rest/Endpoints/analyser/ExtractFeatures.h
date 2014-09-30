#pragma once

#include <iostream>
#include <sstream>
#include <map>
#include "../../FactoryEndpoint.h"
#include "../../../analyser/FactoryAnalyser.h"
#include "../../../analyser/tools/MatrixTools.h"
#include "../../../analyser/tools/TrainTestFeaturesTools.h"
#include "../../../analyser/tools/FrameFilter.h"
#include "../../../analyser/sources/TextFileSourceV2.h"
#include "../../IEndpoint.h"
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>


#include "../../../commons/FileDownloader.h"

#define LABELS_EXT_EXTRACT_FEATURES ".labels"

//http://localhost:9383/analyser?input=/home/amourao/code/shotdetection/sapo_keyframes_sample&analyser=cedd&task=blip&output=blip_cedd
class ExtractFeatures : public IEndpoint
{

public:
	ExtractFeatures();
	ExtractFeatures(string type);
	~ExtractFeatures();

	void* createType(string &typeId);

	void handleRequest(string method, map<string, string> queryStrings, istream&, HTTPServerResponse &resp);

	string getFeatures(map<string, string> parameters);
	string getFeaturesSingle(map<string, string> parameters);


private:
  string type;
  void exportLabels(string filename, vector<map<string, int> > labels);
};

