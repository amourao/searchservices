#include "GenericIndexer.h"

static GenericIndexer mdicalImageClassifierEndpointFactory;

GenericIndexer::GenericIndexer(string type){
	this->type = type; 
}

GenericIndexer::GenericIndexer(){

	FactoryEndpoint::getInstance()->registerType("/genericIndexer",this);

}

GenericIndexer::~GenericIndexer(){}

void* GenericIndexer::createType(string& type){
	std::cout << "New type requested: " << type << std::endl;
	
	if (type == "/genericIndexer"){
		return new GenericIndexer(type);	
	}	
	std::cerr << "Error registering type from constructor (this should never happen)" << std::endl;
	return NULL;
}


void GenericIndexer::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  	if(method != "GET"){
		resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		resp.send();
		return ;
	} 
	if (type == "/genericIndexer"){

		string response("");
		string action = queryStrings["action"];

		if (action == "create"){
			response = create(queryStrings);
		} else if (action == "retrieve"){
			response = retrieve(queryStrings);
		}

		resp.setContentType("application/json");
		std::ostream& out = resp.send();
		out << response;
		out.flush();
	}

}


string GenericIndexer::retrieve(map<string, string> parameters){
	FileDownloader fd;

	
	string filename = fd.getFile(parameters["url"]);
	string analyserName = parameters["analyser"];
	string indexerName = parameters["indexer"];
	string taskName = parameters["task"];
	string retrievalType = parameters["type"];	
	int n = atoi(parameters["n"].c_str());

	
	FactoryAnalyser * f = FactoryAnalyser::getInstance();

	IAnalyser* analyser= (IAnalyser*)f->createType(analyserName);
	
	IDataModel* data = analyser->getFeatures(filename);
	vector<float>* features = (vector<float>*) data->getValue();

	FactoryIndexer * fc = FactoryIndexer::getInstance();
	
	IIndexer* indexer = (IIndexer*)fc->createType(indexerName);
	
	stringstream ss;

	ss << taskName << "_" << analyserName << "_" << indexerName;
	
	indexer->load(ss.str());
	
	vector<std::pair<string,float> > resultList;

	if (retrievalType == "normal"){
		resultList = indexer->knnSearchName(*features,n);
	} else if (retrievalType == "radius"){
		float radius = atof(parameters["radius"].c_str());
		resultList = indexer->radiusSearchName(*features,radius,n);
	}
	
	Json::Value root;
	Json::Value results;
	
	Json::Value featureArray(Json::arrayValue);
	for (int i = 0; i < resultList.size(); i++){
		
		featureArray.append(features->at(i));
		//cout << features->at(i) << " " ;
	}
	
	root["result"] = "ok";
	root["indexer"] = ss.str();
	root["list"] = featureArray;
	stringstream ssJ;
	ssJ << root;
	return ssJ.str();
}

string GenericIndexer::create(map<string, string> parameters){
	FileDownloader fd;
	
	string filename = fd.getFile(parameters["trainData"]);
	string analyserName = parameters["analyser"];
	string indexerName = parameters["indexer"];
	string taskName = parameters["task"];
	
	string file(filename);

	Mat features;
	Mat labels;
	
	MatrixTools::readBin(file, features, labels);

	FactoryIndexer * fc = FactoryIndexer::getInstance();
	IIndexer* indexer = (IIndexer*)fc->createType(indexerName);

	indexer->index(features);

	stringstream ss;

	ss << taskName << "_" << analyserName << "_" << indexerName;

	indexer->save(ss.str());

	Json::Value root;
	Json::Value results;
	
	root["result"] = "ok";
	root["indexer"] = ss.str();
	stringstream ssJ;
	ssJ << root;
	return ssJ.str();
}