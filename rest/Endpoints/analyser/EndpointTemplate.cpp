#include "MedicalImageClassifier.h"

static MedicalImageClassifier mdicalImageClassifierEndpointFactory;

MedicalImageClassifier::MedicalImageClassifier(string type){
	//this->type = type;
	
	if (type == "/MedicalImageClassifier"){

	}
 
}

MedicalImageClassifier::MedicalImageClassifier(){

	FactoryEndpoint::getInstance()->registerType("/MedicalImageClassifier",this);

}

MedicalImageClassifier::~MedicalImageClassifier(){}

void* MedicalImageClassifier::createType(string& type){
	//TODO
	std::cout << "New type requested: " << type << std::endl;
	
	
	if (type == "/MedicalImageClassifier"){
		return new MedicalImageClassifier(type);	
	}	
	std::cerr << "Error registering type from constructor (this should never happen)" << std::endl;
	return NULL;
}


void MedicalImageClassifier::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  	if(method != "GET"){
		resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		resp.send();
		return ;
	}

	if (type == "/MedicalImageClassifier"){

	}

}
