#include "VideoTagger.h"

static VideoTagger VideoTaggerEndpointFactory;

static vector<SVMClassifier*> classifierInstances;

static const vector<double> thresholdsAlt({0.2,0,0.4,0.01,0,0,0});
static const vector<double> thresholds({0.8,0.75,0.7,0.7,0.6,0.7,0.85});

static const vector<string> concepts({"caricas","kizomba","league","minecraft","touro","violetta","zumba"});
static const vector<string> features({"cedd"});

VideoTagger::VideoTagger(string type){
    this->type = type;
}

VideoTagger::VideoTagger(){
	FactoryEndpoint::getInstance()->registerType("/videoTagger",this);
}

VideoTagger::~VideoTagger()
{}

void* VideoTagger::createType(string& type){
	//TODO
	std::cout << "New type requested: " << type << std::endl;

	if (type == "/videoTagger")
		return new VideoTagger(type);

	std::cerr << "Error registering type from constructor (this should never happen)" << std::endl;
	return NULL;
}


void VideoTagger::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  	if(method != "GET"){
		resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		resp.send();
		return ;
	}

	resp.setContentType("application/json");
	std::ostream& out = resp.send();

	std::string response("");

	if (type == "/videoTagger"){
        if (queryStrings["action"] == "tag")
            response = getTags(queryStrings);
		else if (queryStrings["action"] == "upload")
            response = index(queryStrings);

    }

	//std::cout << response << std::endl;
	out << response;
	out.flush();


}

string VideoTagger::getTags(map<string, string > parameters){
    timestamp_type start, end;

    int downloadTime, keyframeTime, feTime, posProcessingTime;

    vector<SVMClassifier*> classifierInstances;
    for(string c: concepts){
        for(string f: features){
            string base = "sapo_" + c + "_" + f;
            SVMClassifier* s = new SVMClassifier(c);
            cout << "loaded " << base << ": " << s->load(base) << endl;
            classifierInstances.push_back(s);
        }
    }

    FactoryAnalyser * f = FactoryAnalyser::getInstance();
    string featuresType = "cedd";
	IAnalyser* analyser= (IAnalyser*)f->createType(featuresType);

	FileDownloader fd;

	string filename = fd.getFile(parameters["url"]);
	int step = 5;

	vector<int> diffs,frames,keyframes,keyframesWithMiddle, keyframesDiffs;
	vector<double> keyframesWithMiddleTimes;
	vector<string> framesPaths;

	ShotDetector s;
	s.detectScenes(filename,step,diffs,frames);
	s.getPeaks(diffs,frames,keyframes,keyframesDiffs);
	s.addMiddleKeyframes(keyframes,keyframesWithMiddle);
	s.writeFrames(filename,keyframesWithMiddle,framesPaths);
	s.convertFramesIndexToTimes(filename,keyframesWithMiddle,keyframesWithMiddleTimes);

    vector<pair<int,vector<string> > > conceptsPerFrame;
    vector<pair<int,vector<string> > > conceptsPerFrameAlt;
    vector<int> conceptsCountPerVideo(classifierInstances.size(),0);
    vector<double> conceptsScorePerVideo(classifierInstances.size(),0);
    vector<vector<double> > confidencePerFrame;

    int analysedFrames = 0;

	for(uint i = 0; i < framesPaths.size(); i++){
        string path = framesPaths.at(i);
        Mat frame = imread(path);

        vector<double> confidences;
        pair<int,vector<string> > frameConcepts (keyframesWithMiddle.at(i),vector<string>());
        pair<int,vector<string> > frameConceptsAlt (keyframesWithMiddle.at(i),vector<string>());
        if(FrameFilter::hasEdges(frame)){
            analysedFrames++;
            Mat features;

            IDataModel* data = analyser->getFeatures(path);
            vector<float>* v = (vector<float>*) data->getValue();
            vector<float> v2 = *v;
            MatrixTools::vectorToMat(v2, features);


            for(uint j = 0; j < classifierInstances.size(); j++){
                SVMClassifier* svm = classifierInstances.at(j);
                float detected = svm->classify(features);
                float conf = -svm->getClassificationConfidence(features);
                confidences.push_back(conf);
                conceptsScorePerVideo.at(j)+=conf;
                if(detected == 1){
                    conceptsCountPerVideo.at(j)++;
                    frameConcepts.second.push_back(svm->getName());
                }
                if(conf >= thresholdsAlt.at(j)){
                    frameConceptsAlt.second.push_back(svm->getName());
                }
            }
		} else {
            frameConcepts.second.push_back("(ignored)");
            frameConceptsAlt.second.push_back("(ignored)");
		}
		conceptsPerFrame.push_back(frameConcepts);
		conceptsPerFrameAlt.push_back(frameConceptsAlt);
		confidencePerFrame.push_back(confidences);

	}

    int totalFrames = framesPaths.size();
    vector<string> detectedConcepts;
    vector<string> detectedConceptsAlt;
    vector<double> detectedRatios;
    vector<double> detectedRatiosAlt;

    vector<double> detectedRatios2;
    vector<double> detectedRatiosAlt2;

    for(uint i = 0; i < conceptsCountPerVideo.size(); i++){
        double ratio = conceptsCountPerVideo.at(i)/double(analysedFrames);
        double ratioAlt = conceptsCountPerVideo.at(i)/double(totalFrames);

        double ratio2 = conceptsScorePerVideo.at(i)/double(analysedFrames);
        double ratioAlt2 = conceptsScorePerVideo.at(i)/double(totalFrames);

        detectedRatios.push_back(ratio);
        detectedRatiosAlt.push_back(ratioAlt);

        detectedRatios2.push_back(ratio2);
        detectedRatiosAlt2.push_back(ratioAlt2);

        if (ratio >= thresholds.at(i)){
            detectedConcepts.push_back(concepts.at(i));
        }

        if (ratio2 >= thresholdsAlt.at(i)){
            detectedConceptsAlt.push_back(concepts.at(i));
        }
    }

	Json::Value root;
	Json::Value results;

	Json::Value conceptArray(Json::arrayValue);

	for (uint i = 0; i < detectedConcepts.size(); i++){
		conceptArray.append(detectedConcepts.at(i));
		//cout << features->at(i) << " " ;
	}

	Json::Value conceptArrayAlt(Json::arrayValue);

	for (uint i = 0; i < detectedConceptsAlt.size(); i++){
		conceptArrayAlt.append(detectedConceptsAlt.at(i));
		//cout << features->at(i) << " " ;
	}



    Json::Value conceptThresholds(Json::arrayValue);

	for (uint i = 0; i < detectedRatios.size(); i++){
        Json::Value concept;
        concept["ratio"] = detectedRatios.at(i);
        concept["ratioAlt"] = detectedRatiosAlt.at(i);

        concept["ratio2"] = detectedRatios2.at(i);
        concept["ratioAlt2"] = detectedRatiosAlt2.at(i);

        concept["concept"] = concepts.at(i);
		conceptThresholds.append(concept);
		//cout << features->at(i) << " " ;
	}

    Json::Value frameArray(Json::arrayValue);
	for (uint i = 0; i < conceptsPerFrame.size(); i++){
        Json::Value frame;

        frame["pos"] = conceptsPerFrame.at(i).first;
        frame["time"] = keyframesWithMiddleTimes.at(i);
        vector<string> spath = StringTools::split(framesPaths.at(i), '/');
        frame["name"] = spath.at(spath.size()-1);

        Json::Value frameConceptArray(Json::arrayValue);
        for(string c: conceptsPerFrame.at(i).second)
            frameConceptArray.append(c);

        frame["concepts"] = frameConceptArray;

        Json::Value frameConceptArrayAlt(Json::arrayValue);
        for(string c: conceptsPerFrameAlt.at(i).second)
            frameConceptArrayAlt.append(c);

        frame["conceptsAlt"] = frameConceptArrayAlt;

        Json::Value frameConfArray(Json::arrayValue);
        for(uint j = 0; j < confidencePerFrame.at(i).size(); j++){
            Json::Value cc;
            cc["concept"] = concepts.at(j);
            cc["score"] = confidencePerFrame.at(i).at(j);
            frameConfArray.append(cc);
        }

        frame["conceptsConfidence"] = frameConfArray;

		//cout << features->at(i) << " " ;

		frameArray.append(frame);
	}
	/*
	for(int i = 0; i < 8; i++)
	{
		Json::Value result;
		Json::Value tags;
		result["id"] = rand() % 25000;
		result["rank"] = rand() % 10;
		result["path"] = "/some/path/";
		result["title"] = "randomTitle";
		tags["0"] = "tag1";
		tags["1"] = "tag2";
		tags["2"] = "tag3";
		result["tags"] = tags;
		results[i] = result;
	}
	*/
	root["url"] = parameters["url"];
	root["localUrl"] = filename;
	vector<string> spath = StringTools::split(filename, '/');
    root["localName"] = spath.at(spath.size()-1);

	root["frames"] = frameArray;
	root["detectedConcepts"] = conceptArray;
	root["detectedConceptsAlt"] = conceptArrayAlt;

	root["conceptThresholds"] = conceptThresholds;
	stringstream ss;
	ss << root;
	return ss.str();
}

string VideoTagger::index(map<string, string > parameters){
    GenericIndexer gi("a");

    string filename = parameters["url"];
    vector<string> framesPaths = StringTools::split(parameters["url"],',');

    Json::Value indexArray(Json::arrayValue);

    for(uint i = 0; i < framesPaths.size(); i++){
        map<string,string> param;

        param["filename"] = "file://" + framesPaths.at(i);
        param["analyser"] = parameters["analyser"];
        param["indexer"] = parameters["indexer"];
        param["task"] = parameters["task"];

        indexArray.append(gi.addToIndexLive(param));
    }

    Json::Value root;
    root["result"] = "ok";
    root["frameIndexes"] = indexArray;
	stringstream ss;
	ss << root;
	return ss.str();
}
