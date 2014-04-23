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

		if (action == "get"){
			response = get(queryStrings);
		}

		resp.setContentType("application/json");

		std::ostream& out = resp.send();

		out << response;
		out.flush();
	}

}


std::string GetMetadata::get(map<string, string> parameters){

    if (!(parameters.count("table") > 0 &&
        parameters.count("fieldsToGet") > 0 &&
        parameters.count("keys") > 0 &&
        parameters.count("values") > 0 &&
        parameters.count("isOr") > 0
        ))
        return "";

    int limit = -1;
    if (parameters.count("limit") > 0)
        limit = atoi(parameters["limit"].c_str());

	vector<map<string,string> > s = DatabaseConnection::getRows(parameters["table"] ,parameters["fieldsToGet"],StringTools::split(parameters["keys"],','),StringTools::split(parameters["values"],','),parameters["isOr"]=="true",limit);

	Json::Value resultArray(Json::arrayValue);

	for(unsigned int i = 0; i < s.size(); i++){
        std::map<std::string, std::string> sp = s.at(i);
		std::map<std::string, std::string>::iterator iter;
        Json::Value node;
        for (iter = sp.begin(); iter != sp.end(); ++iter) {
           node[iter->first] = iter->second;
        }
		resultArray.append(node);
	}
	stringstream ss;
	ss << resultArray;
	return ss.str();
}
