#include "VideoTagger.h"

static VideoTagger VideoTaggerEndpointFactory;

static vector<SVMClassifier*> classifierInstances;

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

	if (type == "/videoTagger")
		response = getTags(queryStrings);

	//std::cout << response << std::endl;
	out << response;
	out.flush();


}

string VideoTagger::getTags(map<string, string > parameters){

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
	vector<string> framesPaths;

	ShotDetector s;
	s.detectScenes(filename,step,diffs,frames);
	s.getPeaks(diffs,frames,keyframes,keyframesDiffs);
	s.addMiddleKeyframes(keyframes,keyframesWithMiddle);
	s.writeFrames(filename,keyframesWithMiddle,framesPaths);

    vector<pair<int,vector<string> > > conceptsPerFrame;
    vector<int> conceptsCountPerVideo(classifierInstances.size(),0);

	for(uint i = 0; i < framesPaths.size(); i++){
        string path = framesPaths.at(i);
        Mat features;

        IDataModel* data = analyser->getFeatures(path);
        vector<float>* v = (vector<float>*) data->getValue();
		vector<float> v2 = *v;
		MatrixTools::vectorToMat(v2, features);

        pair<int,vector<string> > frameConcepts (keyframesWithMiddle.at(i),vector<string>());
		for(uint j = 0; j < classifierInstances.size(); j++){
            SVMClassifier* svm = classifierInstances.at(j);
            float detected = svm->classify(features);
            if(detected == 1){
                conceptsCountPerVideo.at(j)++;
                frameConcepts.second.push_back(svm->getName());
            }
		}
		conceptsPerFrame.push_back(frameConcepts);

	}

    int totalFrames = framesPaths.size();
    vector<string> detectedConcepts;
    vector<double> detectedRatios;

    for(uint i = 0; i < conceptsCountPerVideo.size(); i++){
        double ratio = conceptsCountPerVideo.at(i)/double(totalFrames);
        detectedRatios.push_back(ratio);
        if (ratio >= thresholds.at(i)){
            detectedConcepts.push_back(concepts.at(i));
        }
    }

	Json::Value root;
	Json::Value results;

	Json::Value conceptArray(Json::arrayValue);

	for (uint i = 0; i < detectedConcepts.size(); i++){
		conceptArray.append(detectedConcepts.at(i));
		//cout << features->at(i) << " " ;
	}

    Json::Value conceptThresholds(Json::arrayValue);

	for (uint i = 0; i < detectedRatios.size(); i++){
        Json::Value concept;
        concept["ratio"] = detectedRatios.at(i);
        concept["concept"] = concepts.at(i);
		conceptThresholds.append(concept);
		//cout << features->at(i) << " " ;
	}

    Json::Value frameArray(Json::arrayValue);
	for (uint i = 0; i < conceptsPerFrame.size(); i++){
        Json::Value frame;

        frame["pos"] = conceptsPerFrame.at(i).first;
        frame["path"] = framesPaths.at(i);

        Json::Value frameConceptArray(Json::arrayValue);
        for(string c: conceptsPerFrame.at(i).second)
            frameConceptArray.append(c);

        frame["concepts"] = frameConceptArray;
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
	root["frames"] = frameArray;
	root["detectedConcepts"] = conceptArray;
	root["conceptThresholds"] = conceptThresholds;
	stringstream ss;
	ss << root;
	return ss.str();
}
