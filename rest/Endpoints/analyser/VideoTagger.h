#pragma once

#include <utility>
#include <iostream>
#include <sstream>
#include <map>
#include "../../FactoryEndpoint.h"
#include "../../../analyser/FactoryAnalyser.h"
#include "../../../analyser/nTag/SVMClassifier.h"
#include "../../../analyser/tools/ShotDetector.h"
#include "../../../analyser/tools/FrameFilter.h"
#include "../../../analyser/sources/TextFileSourceV2.h"

#include "../../IEndpoint.h"
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>


#include "../../../commons/FileDownloader.h"

#define LABELS_EXT_EXTRACT_FEATURES ".labels"


class VideoTagger : public IEndpoint
{

public:
	VideoTagger();
	VideoTagger(string type);
	~VideoTagger();

	void* createType(string &typeId);

	void handleRequest(string method, map<string, string> queryStrings, istream&, HTTPServerResponse &resp);

	string getTags(map<string, string> parameters);
private:
  string type;
};

