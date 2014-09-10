#include "LireExtractor.h"

static LireExtractor lireExtractorFactory;

LireExtractor::LireExtractor(){
	FactoryAnalyser::getInstance()->registerType("lireExtractor",this);
}


LireExtractor::LireExtractor(string& _type){
	type = _type;
}

LireExtractor::LireExtractor(string& _type, map<string, string>& params){
	type = _type;

    if (params.size() == 0)
        return;

    algorithm = params["algorithm"];
    host = params["host"];
    endpoint = params["endpoint"];
 	//jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
}

LireExtractor::~LireExtractor(){

}

void* LireExtractor::createType(string& type){
	if (type == "lireExtractor"){
		return new LireExtractor(type);
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* LireExtractor::createType(string& createType, map<string ,string>& params){
    //cout << createType << endl;
	return new LireExtractor(type,params);
}


void LireExtractor::extractFeatures(Mat& src, Mat& dst){
	stringstream ss6;
	ss6 << "/localstore/amourao/code/searchservices/tmpData/" << StringTools::genRandom(12) << ".png";

	string filename = ss6.str();

	imwrite(filename,src);

	vector<float> features;
	extractFeatures(filename, features);

	MatrixTools::vectorToMat(features,dst);
}

void LireExtractor::extractFeatures(string filename, vector<float>& features){
	vector<vector<float> > featuresAll;
	extractFeaturesMulti(filename,featuresAll);
	features = featuresAll.at(0);
}

void LireExtractor::extractFeaturesMulti(string filename, vector<vector<float> >& features){

	// http://localhost:8080/extractor?filename=/home/amourao/Desktop/1752-1947-5-498-2.jpg;/home/amourao/Desktop/1752-1947-5-498-2.jpg;/home/amourao/Desktop/1752-1947-5-498-2.jpg;/home/amourao/Desktop/1752-1947-5-498-2.jpg&extractor=cedd_fcth
	stringstream ss;
	ss << host << "?filename=" << filename << "&extractor=" << algorithm;
	string uriStr = ss.str();
	//cout << uriStr << endl;
	try {
	    // prepare session
	    string request;
	    URI uri(uriStr);
	    HTTPClientSession session(uri.getHost(), uri.getPort());

	    // prepare path
	    string path(uri.getPathAndQuery());
	    if (path.empty()) path = "/";

	    // send request
	    HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
	    session.sendRequest(req);

	    // get response
	    HTTPResponse res;
	    //cout << res.getStatus() << " " << res.getReason() << endl;

	    // print response
	    istream &is = session.receiveResponse(res);

	    Json::Value root;   // will contains the root value after parsing.
	    Json::Reader reader;
	    bool parsingSuccessful = reader.parse( is, root, false );

	    Json::Value featuresJson = root["featureVector"];

	    features.reserve(featuresJson.size());
	    for(int i = 0; i < featuresJson.size(); i++){
	    	vector<float> featuresOne;
	    	featuresOne.reserve(getFeatureVectorSize());
	    	for(int j = 0; j < featuresJson[i].size(); j++){
	    		featuresOne.push_back(featuresJson[i][j].asDouble());
	    	}
	    	features.push_back(featuresOne);
	    }
	} catch (Poco::Exception &ex){
    	cerr << ex.displayText() << endl;
  	}
}


int LireExtractor::getFeatureVectorSize(){
	if( algorithm == "cedd")
		return 144;
	else if(algorithm == "fcth")
		return 192;
	else if (algorithm == "cedd_fcth")
		return 144+192;
	return -1;
}

string LireExtractor::getName(){
	return type;
}
