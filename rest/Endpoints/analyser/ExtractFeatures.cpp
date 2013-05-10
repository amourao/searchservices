#include "ExtractFeatures.h"

static ExtractFeatures extractFeaturesEndpointFactory;

ExtractFeatures::ExtractFeatures(string type){
  this->type = type;
}

ExtractFeatures::ExtractFeatures(){

	FactoryEndpoint::getInstance()->registerType("/analyser",this);

}

ExtractFeatures::~ExtractFeatures()
{}

void* ExtractFeatures::createType(string& type){
	//TODO
	std::cout << "New type requested: " << type << std::endl;
	
	if (type == "/analyser")
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

	//std::cout << response << std::endl;
	out << response;
	out.flush();


}



string ExtractFeatures::getFeatures(map<string, string > parameters){
	
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
	root["results"] = featureArray;
	stringstream ss;
	ss << root;
	return ss.str();
}
