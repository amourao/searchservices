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
		} else if (action == "addToIndex"){
            response = addToIndexLive(queryStrings);
		}

		std::ostream& out = resp.send();

		out << response;
		out.flush();
	}

}


string GenericIndexer::retrieve(map<string, string> parameters){
	cout << "**************** start GENREIRC INDEXER ****************" << endl;
	timestamp_type start, end, totalStart, totalEnd;
	get_timestamp(&totalStart);


	FileDownloader fd;

    vector<string> filenames = StringTools::split(parameters["url"],',');
    string analyserName = parameters["analyser"];
    string indexerName = parameters["indexer"];
    string taskName = parameters["task"];
    string retrievalType = parameters["type"];
    string outputFormat = parameters["output"];
    string outputType = parameters["retrievalOutput"];
    string labelsIndexStr = parameters["labelsIndex"];
    string flabelsIndexStr = parameters["flabelsIndex"];

    int n = atoi(parameters["n"].c_str());

    stringstream ss;
    ss << taskName << "_" << analyserName << "_" << indexerName;

    IIndexer* indexer;
    if (indexerInstances.count(ss.str()) == 0){
    	FactoryIndexer * fc = FactoryIndexer::getInstance();
    	indexer = (IIndexer*)fc->createType(indexerName);
    	get_timestamp(&start);
  	  	indexer->load(ss.str());
		get_timestamp(&end);
    	cout << "loading index: " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
    	indexerInstances[ss.str()] = indexer;
	} else {
		indexer = indexerInstances[ss.str()];
	}

	int labelsIndex = atoi(labelsIndexStr.c_str());
	if (labelsIndex >= 0)
		indexer->chooseLabels(labelsIndex);

	int flabelsIndex = atoi(flabelsIndexStr.c_str());

    FactoryAnalyser * f = FactoryAnalyser::getInstance();
    cout << "get feature extractor" << endl;
    FeatureExtractor* analyser= (FeatureExtractor*)f->createType(analyserName);
    cout << "feature extractor " << analyser << endl;
    cout << "start downloading files" << endl;
    vector<std::pair< vector<string>, vector<float> > > resultLists;

    get_timestamp(&start);
    string localFilenames = "";
    for (uint ii = 0; ii < filenames.size(); ii++){
        string filename = fd.getFile(filenames.at(ii));
        localFilenames += filename;
        if (ii != filenames.size())
        	localFilenames += ",";

    }
    get_timestamp(&end);
    cout << "downloading files : " << timestamp_diff_in_milliseconds(start, end) << " ms " << localFilenames << endl;

    vector<vector<float> > allFeatures;
    get_timestamp(&start);
    analyser->extractFeaturesMulti(localFilenames, allFeatures);
    get_timestamp(&end);
    cout << "get features : " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;

	for (uint ii = 0; ii < allFeatures.size(); ii++){
		vector<float> features = allFeatures.at(ii);
        std::pair< vector<string>, vector<float> > resultList;

        get_timestamp(&start);
        if (retrievalType == "normal" && outputType == "name"){
            resultList = indexer->knnSearchName(features,n,flabelsIndex);
        } else if (retrievalType == "normal" && outputType == "id"){
            std::pair< vector<float>, vector<float> > resultListF = indexer->knnSearchId(features,n,flabelsIndex);
            resultList = idToLabels(resultListF);
        } else if (retrievalType == "radius" && outputType == "name"){
            float radius = atof(parameters["radius"].c_str());
            resultList = indexer->radiusSearchName(features,radius,n,flabelsIndex);
        } else if (retrievalType == "radius" && outputType == "id"){
            float radius = atof(parameters["radius"].c_str());
            std::pair< vector<float>, vector<float> > resultListF = indexer->radiusSearchId(features,radius,n,flabelsIndex);
            resultList = idToLabels(resultListF);
        }
        resultLists.push_back(resultList);
        get_timestamp(&end);
    	cout << "retrieving " << ii << ": " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
    }
    delete analyser;
    string result;
    get_timestamp(&start);
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
		result = ssJ.str();
	} if(outputFormat == "trec"){
		stringstream ssJ;

        for (uint j = 0; j < resultLists.size(); j++){
            std::pair< vector<string>, vector<float> > resultList = resultLists.at(j);
            for (uint i = 0; i < resultList.first.size(); i++){
                ssJ << j << "\t1\t" << resultList.first.at(i) << "\t" << (i+1) << "\t" << "\t" << resultList.second.at(i) << "\t" << taskName << endl;
            }
        }
		result = ssJ.str();
	}
	get_timestamp(&end);
    cout << "formating : " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
    get_timestamp(&totalEnd);
    cout << "**************** total : " << timestamp_diff_in_milliseconds(totalStart, totalEnd) << " ms ****************" << endl;
	return result;

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

	indexer->setFlabels(labels);
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

string GenericIndexer::addToIndexLive(map<string, string> parameters){
	FileDownloader fd;
	timestamp_type start, end, totalStart, totalEnd;


	string filename = parameters["filename"];
	string analyserName = parameters["analyser"];
	string indexerName = parameters["indexer"];
	string taskName = parameters["task"];

    stringstream ss;
    ss << taskName << "_" << analyserName << "_" << indexerName;

    IIndexer* indexer;
    if (indexerInstances.count(ss.str()) == 0){
    	FactoryIndexer * fc = FactoryIndexer::getInstance();
    	indexer = (IIndexer*)fc->createType(indexerName);
    	get_timestamp(&start);
  	  	indexer->load(ss.str());
		get_timestamp(&end);
    	cout << "loading index: " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
    	indexerInstances[ss.str()] = indexer;
	} else {
		indexer = indexerInstances[ss.str()];
	}

    FactoryAnalyser * f = FactoryAnalyser::getInstance();
    cout << "get feature extractor" << endl;
    FeatureExtractor* analyser= (FeatureExtractor*)f->createType(analyserName);
    cout << "feature extractor " << analyser << endl;
    cout << "start downloading files" << endl;
    get_timestamp(&end);

    Mat features;
    get_timestamp(&start);

    string localfile = fd.getFile(filename);
    analyser->extractFeatures(localfile, features);
    get_timestamp(&end);
    cout << "get features : " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;


    stringstream ss2;

	ss2 << taskName << "_" << analyserName << "_" << indexerName;


	indexer->addToIndexLive(features);
	indexer->save(ss2.str());


	Json::Value root;
	Json::Value results;

	root["result"] = "ok";
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
