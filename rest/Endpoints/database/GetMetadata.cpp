#include "GetMetadata.h"

static GetMetadata mdicalImageClassifierEndpointFactory;

GetMetadata::GetMetadata(string type){
	this->type = type;
}

GetMetadata::GetMetadata(){

	FactoryEndpoint::getInstance()->registerType("/getMetadata",this);

}

GetMetadata::~GetMetadata(){}

void* GetMetadata::createType(string& type){
	std::cout << "New type requested: " << type << std::endl;

	if (type == "/getMetadata"){
		return new GetMetadata(type);
	}
	std::cerr << "Error registering type from constructor (this should never happen)" << std::endl;
	return NULL;
}


void GetMetadata::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  	if(method != "GET"){
		resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		resp.send();
		return ;
	}
	if (type == "/getMetadata"){

		string response("");
		resp.setContentType("application/json");

		string action = queryStrings["action"];

		if (action == "create"){
			//response = create(queryStrings);
		} else if (action == "retrieve"){
			//response = retrieve(queryStrings);

			if(queryStrings["output"] == "json")
				resp.setContentType("application/json");
			else if(queryStrings["output"] == "trec")
				resp.setContentType("text/plain");
		}

		std::ostream& out = resp.send();

		out << response;
		out.flush();
	}

}
