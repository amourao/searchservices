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
		resp.setContentType("application/json");

		string action = queryStrings["action"];

		if (action == "create"){
			response = create(queryStrings);
		} else if (action == "retrieve"){
			response = retrieve(queryStrings);
			
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


string GenericIndexer::retrieve(map<string, string> parameters){
	FileDownloader fd;

	
	string filename = fd.getFile(parameters["url"]);
	string analyserName = parameters["analyser"];
	string indexerName = parameters["indexer"];
	string taskName = parameters["task"];
	string retrievalType = parameters["type"];
	string outputFormat = parameters["output"];
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
	

	if(outputFormat == "json"){

		Json::Value root;
		Json::Value results;
		
		Json::Value featureArray(Json::arrayValue);
		Json::Value distArray(Json::arrayValue);
		for (int i = 0; i < resultList.size(); i++){
			featureArray.append(resultList.at(i).first);
			distArray.append(resultList.at(i).second);
		}
		
		root["result"] = "ok";
		root["indexer"] = ss.str();
		root["filename"] = filename;
		root["analyser"] = parameters["analyser"];
		root["indexer"] = parameters["indexer"];
		root["task"] = parameters["task"];
		root["type"] = parameters["type"];	
		root["n"] = atoi(parameters["n"].c_str());
		root["n_true"] = (int)resultList.size();
		if (retrievalType == "radius")
			root["n"] = atof(parameters["radius"].c_str());
		root["idList"] = featureArray;
		root["distList"] = distArray;

		stringstream ssJ;
		ssJ << root;
		return ssJ.str();
	} if(outputFormat == "trec"){
		stringstream ssJ;

		for (int i = 0; i < resultList.size(); i++){
			ssJ << "0\t1\t" << resultList.at(i).first << "\t" << (i+1) << "\t" << "\t" << resultList.at(i).second << "\t" << taskName << endl;
		}
		return ssJ.str();
	}


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
	root["indexer"] = parameters["indexer"];
	root["task"] = parameters["task"];
	stringstream ssJ;
	ssJ << root;
	return ssJ.str();
}