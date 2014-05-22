#include "GenericIndexer.h"

static GenericIndexer indexerEndpointFactory;

static map<string,IIndexer*> indexerInstances;

GenericIndexer::GenericIndexer(string type){
	this->type = type;
}

GenericIndexer::GenericIndexer(){

	FactoryEndpoint::getInstance()->registerType("/genericIndexer",this);

}

//http://ariadne:9383/genericIndexer?action=create&trainData=/home/amourao/data/blip/blip_gist883_fixed.bin&analyser=gist883&indexer=linearIndexer&task=blip
//ariadne:9383/genericIndexer?action=retrieve&type=normal&analyser=gist883&indexer=linearIndexer&task=blip&output=json&url=http://andremourao.com/wp-content/uploads/2013/07/cropped-passe_square.png&n=100
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

    vector<string> filenames = StringTools::split(parameters["url"],',');
    string analyserName = parameters["analyser"];
    string indexerName = parameters["indexer"];
    string taskName = parameters["task"];
    string retrievalType = parameters["type"];
    string outputFormat = parameters["output"];
    string outputType = parameters["retrievalOutput"];

    int n = atoi(parameters["n"].c_str());

    stringstream ss;
    ss << taskName << "_" << analyserName << "_" << indexerName;


    IIndexer* indexer;
    if (indexerInstances.count(ss.str()) == 0){
    	FactoryIndexer * fc = FactoryIndexer::getInstance();
    	indexer = (IIndexer*)fc->createType(indexerName);
  	  	indexer->load(ss.str());
  	  	indexerInstances[ss.str()] = indexer;
	} else {
		indexer = indexerInstances[ss.str()];
	}

    FactoryAnalyser * f = FactoryAnalyser::getInstance();
    IAnalyser* analyser= (IAnalyser*)f->createType(analyserName);

    vector<std::pair< vector<string>, vector<float> > > resultLists;

    for (uint ii = 0; ii < filenames.size(); ii++){
        string filename = fd.getFile(filenames.at(ii));
        IDataModel* data = analyser->getFeatures(filename);
        vector<float>* features = (vector<float>*) data->getValue();
        std::pair< vector<string>, vector<float> > resultList;

        if (retrievalType == "normal" && outputType == "name"){
            resultList = indexer->knnSearchName(*features,n);
        } else if (retrievalType == "normal" && outputType == "id"){
            std::pair< vector<float>, vector<float> > resultListF = indexer->knnSearchId(*features,n);
            resultList = idToLabels(resultListF);
        } else if (retrievalType == "radius" && outputType == "name"){
            float radius = atof(parameters["radius"].c_str());
            resultList = indexer->radiusSearchName(*features,radius,n);
        } else if (retrievalType == "radius" && outputType == "id"){
            float radius = atof(parameters["radius"].c_str());
            std::pair< vector<float>, vector<float> > resultListF = indexer->radiusSearchId(*features,radius,n);
            resultList = idToLabels(resultListF);
        }
        resultLists.push_back(resultList);

    }
	if(outputFormat == "json"){

		Json::Value root;
		Json::Value results;

        Json::Value images(Json::arrayValue);
		for (uint j = 0; j < resultLists.size(); j++){
            Json::Value featureArray(Json::arrayValue);
            Json::Value distArray(Json::arrayValue);
            std::pair< vector<string>, vector<float> > resultList = resultLists.at(j);
            for (uint i = 0; i < resultList.first.size(); i++){
            	Json::Value tmp;
            	tmp["id"] = resultList.first.at(i);
                featureArray.append(tmp);
                distArray.append(resultList.second.at(i));
			}
			Json::Value pairArray;
			pairArray["filename"] = filenames.at(j);
			pairArray["n_true"] = (int)resultList.first.size();
            pairArray["idList"] = featureArray;
            pairArray["distList"] = distArray;
            images.append(pairArray);
		}

		root["result"] = "ok";
		root["results"] = images;
		root["indexer"] = ss.str();
		root["analyser"] = parameters["analyser"];
		root["indexer"] = parameters["indexer"];
		root["task"] = parameters["task"];
		root["type"] = parameters["type"];
		root["n"] = atoi(parameters["n"].c_str());
		if (retrievalType == "radius")
            root["n"] = atoi(parameters["radius"].c_str());


		stringstream ssJ;
		ssJ << root;
		return ssJ.str();
	} if(outputFormat == "trec"){
		stringstream ssJ;

        for (uint j = 0; j < resultLists.size(); j++){
            std::pair< vector<string>, vector<float> > resultList = resultLists.at(j);
            for (uint i = 0; i < resultList.first.size(); i++){
                ssJ << j << "\t1\t" << resultList.first.at(i) << "\t" << (i+1) << "\t" << "\t" << resultList.second.at(i) << "\t" << taskName << endl;
            }
        }
		return ssJ.str();
	}

    return "";
}

string GenericIndexer::create(map<string, string> parameters){
	FileDownloader fd;

	string filename = parameters["trainData"];
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

std::pair< vector<string>, vector<float> > GenericIndexer::idToLabels(std::pair< vector<float>, vector<float> > v1){
		vector<string> result;

		for(uint i = 0; i < v1.first.size(); i++){
				stringstream ss;
				ss << v1.first.at(i);
				string s = ss.str();
				result.push_back(s);
		}

		return make_pair(result,v1.second);
	}
