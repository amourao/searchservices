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
//ariadne:9383/genericIndexer?action=retrieve&type=normal&analyser=gist883&indexer=linearIndexer&task=blip&output=json&url=http://andremourao.com/wp-content/uploads/2013/07/cropped-passe_square.png&n=100&labelsIndex=-1&flabelsIndex=0&search_limit=0.1&retrievalOutput=id
GenericIndexer::~GenericIndexer(){
    /*map<string, IIndexer*>::iterator iter;
    for(iter = indexerInstances.begin(); iter != indexerInstances.end(); iter++){
        IIndexer* i = iter->second;
        delete i;
    }
    */
}

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

    Json::Value root;
    root["result"] = "error";
    root["code"] = "7";
    root["description"] = "Unknown error";
    stringstream ss;
    ss << root;
    string response = ss.str();
    root = Json::Value();


    try {

        if(method != "GET" && method != "POST"){
            resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            resp.send();
        } else if (method == "POST" && type == "/genericIndexer"){
            resp.setContentType("application/json");

            string action = queryStrings["action"];

            if (action == "retrieveExtracted"){
                response = retrieveExtracted(queryStrings,input);
            }

        } else if (type == "/genericIndexer"){
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
            } else if (action == "saveIndex"){
                response = saveIndex(queryStrings);
            }
        }
    } catch(exception& e) {
        root["exception"] = e.what();
        root["result"] = "error";
        root["code"] = "7";
        root["description"] = "Unknown error";
        stringstream ss2;
        ss2 << root;
        response = ss2.str();
    } catch(...) {
        root["exception"] = "(Unknown)";
        root["result"] = "error";
        root["code"] = "7";
        root["description"] = "Unknown error";
        stringstream ss2;
        ss2 << root;
        response = ss2.str();
    }

    std::ostream& out = resp.send();
    out << response;
    out.flush();


}

IIndexer* GenericIndexer::getIndexer(string indexerName,string s){
    timestamp_type start, end;
    IIndexer* indexer;
    if (indexerInstances.count(s) == 0){
        FactoryIndexer * fc = FactoryIndexer::getInstance();
        indexer = (IIndexer*)fc->createType(indexerName);
        get_timestamp(&start);
        indexer->load(s);
        get_timestamp(&end);
        cout << "loading index: " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
        indexerInstances[s] = indexer;
    } else {
        indexer = indexerInstances[s];
    }
    return indexer;
}

string GenericIndexer::retrieve(map<string, string> parameters){
	cout << "**************** start GENERIC INDEXER ****************" << endl;
	cout << "**************** action retrieve ****************" << endl;
	timestamp_type start, end, totalStart, totalEnd, tmpStart, tmpEnd;
	uint setupTime, downloadTime, feTime, retrievalTime, formatingTime;
	get_timestamp(&totalStart);
	get_timestamp(&start);


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

    double search_limit = 1;

    if (parameters.count("search_limit") > 0)
        search_limit = std::stod(parameters["search_limit"]);

    stringstream ss;
    ss << taskName << "_" << analyserName << "_" << indexerName;

    IIndexer* indexer = getIndexer(indexerName,ss.str());


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


    get_timestamp(&end);
    setupTime = timestamp_diff_in_milliseconds(start, end);
    get_timestamp(&start);
    string localFilenames = "";
    for (uint ii = 0; ii < filenames.size(); ii++){
        string filename = fd.getFile(filenames.at(ii));
        localFilenames += filename;
        if (ii != filenames.size())
        	localFilenames += ",";

    }
    get_timestamp(&end);
    downloadTime = timestamp_diff_in_milliseconds(start, end);
    cout << "downloading files : " << timestamp_diff_in_milliseconds(start, end) << " ms " << localFilenames << endl;
    get_timestamp(&start);


    vector<vector<float> > allFeatures;
    get_timestamp(&start);
    analyser->extractFeaturesMulti(localFilenames, allFeatures);
    get_timestamp(&end);
    cout << "get features : " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
    feTime = timestamp_diff_in_milliseconds(start, end);

    get_timestamp(&start);
	for (uint ii = 0; ii < allFeatures.size(); ii++){
		vector<float> features = allFeatures.at(ii);
        std::pair< vector<string>, vector<float> > resultList;

        get_timestamp(&tmpStart);
        if (retrievalType == "normal" && outputType == "name"){
            resultList = indexer->knnSearchName(features,n,flabelsIndex, search_limit);
        } else if (retrievalType == "normal" && outputType == "id"){
            std::pair< vector<float>, vector<float> > resultListF = indexer->knnSearchId(features,n,flabelsIndex, search_limit);
            resultList = idToLabels(resultListF);
        } else if (retrievalType == "radius" && outputType == "name"){
            float radius = atof(parameters["radius"].c_str());
            resultList = indexer->radiusSearchName(features,radius,n,flabelsIndex, search_limit);
        } else if (retrievalType == "radius" && outputType == "id"){
            float radius = atof(parameters["radius"].c_str());
            std::pair< vector<float>, vector<float> > resultListF = indexer->radiusSearchId(features,radius,n,flabelsIndex, search_limit);
            resultList = idToLabels(resultListF);
        }
        resultLists.push_back(resultList);
        get_timestamp(&tmpEnd);
        cout << "retrieve " << ii  << ": " << timestamp_diff_in_milliseconds(tmpStart, tmpEnd) << " ms" << endl;

    }
    get_timestamp(&end);
    cout << "retrieval: " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
    retrievalTime = timestamp_diff_in_milliseconds(start, end);
    get_timestamp(&start);
    delete analyser;
    string result;
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

        get_timestamp(&end);
        cout << "formating : " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
        formatingTime = timestamp_diff_in_milliseconds(start, end);

        Json::Value times;

        times["0: totalTime"] = timestamp_diff_in_milliseconds(totalStart, end);
        times["1: setupTime"] = setupTime;
        times["2: downloadTime"] = downloadTime;
        times["3: feTime"] = feTime;
        times["4: retTime"] = retrievalTime;
        times["5: formatingTime"] = formatingTime;


        root["times"] = times;

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

    get_timestamp(&totalEnd);




    cout << "**************** total : " << timestamp_diff_in_milliseconds(totalStart, totalEnd) << " ms ****************" << endl;
    cout << "**************** end GENERIC INDEXER ****************" << endl;
	return result;

}



string GenericIndexer::retrieveExtracted(map<string, string> parameters, istream& input){
	cout << "**************** start GENERIC INDEXER ****************" << endl;
	cout << "**************** action retrieve ****************" << endl;
	timestamp_type start, end, totalStart, totalEnd, tmpStart, tmpEnd;
	uint setupTime, downloadTime, feTime, retrievalTime, formatingTime;
	get_timestamp(&totalStart);
	get_timestamp(&start);

    std::string dataString;
    Poco::StreamCopier::copyToString(input, dataString); // save returned data for further process

    cout << parameters["action"] << " " << dataString << endl;
    return "";

    /*
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

    double search_limit = 1;

    if (parameters.count("search_limit") > 0)
        search_limit = std::stod(parameters["search_limit"]);

    stringstream ss;
    ss << taskName << "_" << analyserName << "_" << indexerName;

    IIndexer* indexer = getIndexer(indexerName,ss.str());


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


    get_timestamp(&end);
    setupTime = timestamp_diff_in_milliseconds(start, end);
    get_timestamp(&start);
    string localFilenames = "";
    for (uint ii = 0; ii < filenames.size(); ii++){
        string filename = fd.getFile(filenames.at(ii));
        localFilenames += filename;
        if (ii != filenames.size())
        	localFilenames += ",";

    }
    get_timestamp(&end);
    downloadTime = timestamp_diff_in_milliseconds(start, end);
    cout << "downloading files : " << timestamp_diff_in_milliseconds(start, end) << " ms " << localFilenames << endl;
    get_timestamp(&start);


    vector<vector<float> > allFeatures;
    get_timestamp(&start);
    analyser->extractFeaturesMulti(localFilenames, allFeatures);
    get_timestamp(&end);
    cout << "get features : " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
    feTime = timestamp_diff_in_milliseconds(start, end);

    get_timestamp(&start);
	for (uint ii = 0; ii < allFeatures.size(); ii++){
		vector<float> features = allFeatures.at(ii);
        std::pair< vector<string>, vector<float> > resultList;

        get_timestamp(&tmpStart);
        if (retrievalType == "normal" && outputType == "name"){
            resultList = indexer->knnSearchName(features,n,flabelsIndex, search_limit);
        } else if (retrievalType == "normal" && outputType == "id"){
            std::pair< vector<float>, vector<float> > resultListF = indexer->knnSearchId(features,n,flabelsIndex, search_limit);
            resultList = idToLabels(resultListF);
        } else if (retrievalType == "radius" && outputType == "name"){
            float radius = atof(parameters["radius"].c_str());
            resultList = indexer->radiusSearchName(features,radius,n,flabelsIndex, search_limit);
        } else if (retrievalType == "radius" && outputType == "id"){
            float radius = atof(parameters["radius"].c_str());
            std::pair< vector<float>, vector<float> > resultListF = indexer->radiusSearchId(features,radius,n,flabelsIndex, search_limit);
            resultList = idToLabels(resultListF);
        }
        resultLists.push_back(resultList);
        get_timestamp(&tmpEnd);
        cout << "retrieve " << ii  << ": " << timestamp_diff_in_milliseconds(tmpStart, tmpEnd) << " ms" << endl;

    }
    get_timestamp(&end);
    cout << "retrieval: " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
    retrievalTime = timestamp_diff_in_milliseconds(start, end);
    get_timestamp(&start);
    delete analyser;
    string result;
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

        get_timestamp(&end);
        cout << "formating : " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
        formatingTime = timestamp_diff_in_milliseconds(start, end);

        Json::Value times;

        times["0: totalTime"] = timestamp_diff_in_milliseconds(totalStart, end);
        times["1: setupTime"] = setupTime;
        times["2: downloadTime"] = downloadTime;
        times["3: feTime"] = feTime;
        times["4: retTime"] = retrievalTime;
        times["5: formatingTime"] = formatingTime;


        root["times"] = times;

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

    get_timestamp(&totalEnd);




    cout << "**************** total : " << timestamp_diff_in_milliseconds(totalStart, totalEnd) << " ms ****************" << endl;
    cout << "**************** end GENERIC INDEXER ****************" << endl;
	return result;
	*/

}


string GenericIndexer::create(map<string, string> parameters){
    cout << "**************** start GENERIC INDEXER ****************" << endl;
	cout << "**************** action create ****************" << endl;
	timestamp_type totalStart, totalEnd;
	get_timestamp(&totalStart);

	FileDownloader fd;

	string filename = parameters["trainData"];
	string analyserName = parameters["analyser"];
	string indexerName = parameters["indexer"];
	string taskName = parameters["task"];

	string file(filename);

	Mat features;
	Mat labels;

	MatrixTools::readBin(file, features, labels);

    stringstream ss;

	ss << taskName << "_" << analyserName << "_" << indexerName;

	FactoryIndexer * fc = FactoryIndexer::getInstance();
	IIndexer* indexer = (IIndexer*)fc->createType(indexerName);

	indexer->index(features);

	indexer->setFlabels(labels);
	indexer->save(ss.str());
	indexer->loadLabels(ss.str());

	Json::Value root;
	Json::Value results;

	root["result"] = "ok";
	root["indexer"] = ss.str();
	root["indexer"] = parameters["indexer"];
	root["task"] = parameters["task"];

	get_timestamp(&totalEnd);
    cout << "**************** total : " << timestamp_diff_in_milliseconds(totalStart, totalEnd) << " ms ****************" << endl;
	cout << "**************** end GENERIC INDEXER ****************" << endl;
	stringstream ssJ;
	ssJ << root;
	return ssJ.str();
}

string GenericIndexer::addToIndexLive(map<string, string> parameters){
	FileDownloader fd;
    cout << "**************** start GENERIC INDEXER ****************" << endl;
	cout << "**************** action addLive ****************" << endl;
	timestamp_type start, end, totalStart, totalEnd;
	get_timestamp(&totalStart);

	string filename = parameters["filename"];
	string analyserName = parameters["analyser"];
	string indexerName = parameters["indexer"];
	string taskName = parameters["task"];
	string labels = parameters["labels"];

    stringstream ss;
    ss << taskName << "_" << analyserName << "_" << indexerName;

    IIndexer* indexer = getIndexer(indexerName,ss.str());

    FactoryAnalyser * f = FactoryAnalyser::getInstance();
    cout << "get feature extractor" << endl;
    FeatureExtractor* analyser= (FeatureExtractor*)f->createType(analyserName);
    cout << "feature extractor " << analyser << endl;
    cout << "start downloading files" << endl;
    get_timestamp(&end);

    Mat features;
    get_timestamp(&start);
    analyser->extractFeatures(filename, features);
    get_timestamp(&end);
    cout << "get features : " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;


    stringstream ss2;

	ss2 << taskName << "_" << analyserName << "_" << indexerName;


	int flannIndex = indexer->addToIndexLive(features);
	vector<string> labelsS;
	labelsS.push_back(labels);

	vector<string> labelsS2 = StringTools::split(labels,',');
	for(string ii: labelsS2)
        cout << ii << " ";
    cout << endl;
	indexer->addLabelLive(flannIndex,labelsS);
	//indexer->save(ss2.str());


	Json::Value root;
	Json::Value results;

	root["result"] = "ok";
	root["flannIndex"] = flannIndex;

    get_timestamp(&totalEnd);
    cout << "**************** total : " << timestamp_diff_in_milliseconds(totalStart, totalEnd) << " ms ****************" << endl;
	cout << "**************** end GENERIC INDEXER ****************" << endl;

	stringstream ssJ;
	ssJ << root;

	return ssJ.str();
}

string GenericIndexer::saveIndex(map<string, string> parameters){
    FileDownloader fd;
    cout << "**************** start GENERIC INDEXER ****************" << endl;
    cout << "**************** action saveIndex ****************" << endl;
    timestamp_type start, end, totalStart, totalEnd;
    get_timestamp(&totalStart);

    string analyserName = parameters["analyser"];
    string indexerName = parameters["indexer"];
    string taskName = parameters["task"];

    stringstream ss;

    ss << taskName << "_" << analyserName << "_" << indexerName;
    string indexName = ss.str();
    IIndexer* indexer = getIndexer(indexerName,indexName);

    cout << "saving index" << endl;
    indexer->save(indexName);
    cout << "index saved" << endl;

    Json::Value root;
    root["result"] = "ok";
    root["indexName"] = indexName;

    get_timestamp(&totalEnd);
    cout << "**************** total : " << timestamp_diff_in_milliseconds(totalStart, totalEnd) << " ms ****************" << endl;
    cout << "**************** end GENERIC INDEXER ****************" << endl;

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
