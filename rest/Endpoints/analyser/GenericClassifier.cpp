#include "GenericClassifier.h"

static GenericClassifier mdicalImageClassifierEndpointFactory;

GenericClassifier::GenericClassifier(string type){
	this->type = type; 
}

GenericClassifier::GenericClassifier(){

	FactoryEndpoint::getInstance()->registerType("/GenericClassifier",this);

}

GenericClassifier::~GenericClassifier(){}

void* GenericClassifier::createType(string& type){
	std::cout << "New type requested: " << type << std::endl;
	
	
	if (type == "/GenericClassifier"){
		return new GenericClassifier(type);	
	}	
	std::cerr << "Error registering type from constructor (this should never happen)" << std::endl;
	return NULL;
}


void GenericClassifier::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  	if(method != "GET"){
		resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		resp.send();
		return ;
	} 
	if (type == "/GenericClassifier"){

		string response("");
		string action = queryStrings["action"];

		if (action == "create"){
			response = create(queryStrings);
		} else if (action == "classify"){
			response = classify(queryStrings);
		}

		resp.setContentType("application/json");
		std::ostream& out = resp.send();
		out << response;
		out.flush();
	}

}


string GenericClassifier::classify(map<string, string> parameters){
	FileDownloader fd;
	
	string filename = fd.getFile(parameters["url"]);
	string analyserName = parameters["analyser"];
	string classifierName = parameters["classifier"];
	string taskName = parameters["task"];
	
	FactoryAnalyser * f = FactoryAnalyser::getInstance();

	IAnalyser* analyser= (IAnalyser*)f->createType(analyserName);
	
	IDataModel* data = analyser->getFeatures(filename);
	vector<float>* features = (vector<float>*) data->getValue();

	IClassifier* classifier;

	//TODO: add to factory
	if (classifierName == "svm"){
		classifier = new SVMClassifier();
	} else if (classifierName == "knn"){
		classifier = new kNNClassifier();
	} else if (classifierName == "vw"){
		classifier = new VWBasicClassifier();
	} else if (classifierName == "sr"){
		classifier = new SRClassifier();
	}

	stringstream ss;

	ss << taskName << "_" << analyserName << "_" << classifierName;

	classifier->load(ss.str());

	float result = classifier->classify(*features);

	Json::Value root;
	Json::Value results;
	
	root["result"] = "ok";
	root["classifier"] = ss.str();
	root["label"] = result;
	stringstream ssJ;
	ssJ << root;
	return ssJ.str();
}

string GenericClassifier::create(map<string, string> parameters){
	FileDownloader fd;
	
	string filename = fd.getFile(parameters["trainData"]);
	string analyserName = parameters["analyser"];
	string classifierName = parameters["classifier"];
	string taskName = parameters["task"];
	
	string file(filename);

	Mat features;
	Mat labels;
	
	MatrixTools::readBin(file, features, labels);

	IClassifier* classifier;

	//TODO: add to factory
	if (classifierName == "svm"){
		classifier = new SVMClassifier();
	} else if (classifierName == "knn"){
		classifier = new kNNClassifier();
	} else if (classifierName == "vw"){
		classifier = new VWBasicClassifier();
	} else if (classifierName == "sr"){
		classifier = new SRClassifier();
	}

	classifier->train(features,labels.col(0));

	stringstream ss;

	ss << taskName << "_" << analyserName << "_" << classifierName;

	classifier->save(ss.str());

	Json::Value root;
	Json::Value results;
	
	root["result"] = "ok";
	root["classifier"] = ss.str();
	stringstream ssJ;
	ssJ << root;
	return ssJ.str();
}