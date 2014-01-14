#include "ExtractFeatures.h"

static ExtractFeatures extractFeaturesEndpointFactory;

ExtractFeatures::ExtractFeatures(string type){
  this->type = type;
}

ExtractFeatures::ExtractFeatures(){

	FactoryEndpoint::getInstance()->registerType("/analyser",this);
	FactoryEndpoint::getInstance()->registerType("/analyserSingleFile",this);

}

ExtractFeatures::~ExtractFeatures()
{}

void* ExtractFeatures::createType(string& type){
	//TODO
	std::cout << "New type requested: " << type << std::endl;
	
	if (type == "/analyser" || (type == "/analyserSingleFile"))
		return new ExtractFeatures(type);
		
	std::cerr << "Error registering type from constructor (this should never happen)" << std::endl;
	return NULL;
}


void ExtractFeatures::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  	if(method != "GET"){
		resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		resp.send();
		return ;
	}

	resp.setContentType("application/json");
	std::ostream& out = resp.send();

	std::string response("");

	if (type == "/analyser")
		response = getFeatures(queryStrings);
	else if (type == "/analyserSingleFile")
		response = getFeaturesSingle(queryStrings);

	//std::cout << response << std::endl;
	out << response;
	out.flush();


}

string ExtractFeatures::getFeaturesSingle(map<string, string > parameters){
	
	FileDownloader fd;
	
	string filename = fd.getFile(parameters["url"]);
	string extractorName = parameters["extractor"];
	
	
	FactoryAnalyser * f = FactoryAnalyser::getInstance();

	f->listTypes();
	IAnalyser* extractor= (IAnalyser*)f->createType(extractorName);
	IDataModel* data = extractor->getFeatures(filename);
	vector<float>* features = (vector<float>*) data->getValue();
	
	for (int i = 0; i < features->size(); i++){
		cout << features->at(i) << " " ;
	}
	cout << endl;

	Json::Value root;
	Json::Value results;
	
	Json::Value featureArray(Json::arrayValue);
	for (int i = 0; i < features->size(); i++){
		
		featureArray.append(features->at(i));
		//cout << features->at(i) << " " ;
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
	root["result"] = featureArray;
	stringstream ss;
	ss << root;
	return ss.str();
}

string ExtractFeatures::getFeatures(map<string, string > parameters){
	
	FileDownloader fd;
	
	string filename = parameters["input"];
	string analyserName = parameters["analyser"];
	string taskName = parameters["task"];
	string outputLocation = parameters["output"];
	
	
	FactoryAnalyser * f = FactoryAnalyser::getInstance();

	IAnalyser* analyser= (IAnalyser*)f->createType(analyserName);

	Mat src, features, labels;
	
	TextFileSource is(filename);
	cout << is.getImageCount() << endl;
	for (int k = 0; k < is.getImageCount(); k++) {
		if (!(src = is.nextImage()).empty()) { // src contains the image, but the IAnalyser interface needs a path 
			cv::Mat featuresRow;


			cv::Mat label(1, 2, CV_32F);

			//parse the label from the info in the txt file
			string path, id1, id2;

			stringstream liness(is.getImageInfo());

			getline(liness, path, ';');
			getline(liness, id1, ';');
			getline(liness, id2);

			label.at<float>(0, 0) = atoi(id2.c_str());
			label.at<float>(0, 1) = atoi(id1.c_str());

			//cout << path << endl;
			IDataModel* data = analyser->getFeatures(path);
			vector<float>* v = (vector<float>*) data->getValue();
			vector<float> v2 = *v;
			MatrixTools::vectorToMat(v2, featuresRow);
			
			//cout << featuresRow << endl;
			// add the features to the main feature and label matrix
			features.push_back(featuresRow);
			labels.push_back(label);
		}
	}

	features.convertTo(features,CV_32F);
	labels.convertTo(labels,CV_32F);



	MatrixTools::writeBin(outputLocation,features,labels);
	/*
	for(int i = 0; i < 8; i++)
	{
		
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
	Json::Value root;
	root["result"] = "ok";
	root["location"] = outputLocation;
	stringstream ss;
	ss << root;
	return ss.str();
}
