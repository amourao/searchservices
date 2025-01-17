#include "GenericClassifier.h"

static GenericClassifier mdicalImageClassifierEndpointFactory;

GenericClassifier::GenericClassifier(string type){
	this->type = type;
}

GenericClassifier::GenericClassifier(){

	FactoryEndpoint::getInstance()->registerType("/genericClassifier",this);

}

GenericClassifier::~GenericClassifier(){}

void* GenericClassifier::createType(string& type){
	std::cout << "New type requested: " << type << std::endl;


	if (type == "/genericClassifier"){
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
	if (type == "/genericClassifier"){

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


//http://localhost:9083/genericClassifier?action=classify&url=file:///home/amourao/code/searchservices/out.png&analyser=cedd_fcth&classifier=NovaMedCategoryClass
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

	FactoryClassifier * fc = FactoryClassifier::getInstance();
	IClassifier* classifier = (IClassifier*)fc->createType(classifierName);

	float result = classifier->classify(*features);

	Json::Value root;
	Json::Value results;

	root["result"] = "ok";
	root["classifier"] = classifier->getName();
	root["label"] = result;
	stringstream ssJ;
	ssJ << root;
	return ssJ.str();
}
//http://localhost:9898/genericClassifier?action=create&trainData=file:///home/amourao/data_facialExpression_lbp.bin&analyser=LBP&classifier=VWBasicClassifier&task=facialExpressionInter
string GenericClassifier::create(map<string, string> parameters){
	FileDownloader fd;

	string file = fd.getFile(parameters["trainData"]);
	string analyserName = parameters["analyser"];
	string classifierName = parameters["classifier"];
	string taskName = parameters["task"];

	Mat features;
	Mat labels;

	MatrixTools::readBin(file, features, labels);

	FactoryClassifier * fc = FactoryClassifier::getInstance();
	IClassifier* classifier = (IClassifier*)fc->createType(classifierName);

	cout << features.cols << " " << features.rows << " " << labels.cols << " " << labels.rows << endl;

	classifier->train(features,labels.col(1));

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
