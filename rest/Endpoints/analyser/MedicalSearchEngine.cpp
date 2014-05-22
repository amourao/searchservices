#include "MedicalSearchEngine.h"

static MedicalSearchEngine medicalSearchEngineEndpointFactory;

MedicalSearchEngine::MedicalSearchEngine(string type){
	//flannIndex = NULL;
	this->type = type;


	//importIclefData("./rest/database2.txt");
	//readTrainingData("/home/amourao/iclef2013Features/SegHistogramLBP/");
	//createKnnIndex();
	//extractorName = "SEG_LBP_HIST";
	if (type == "/medicalSearch"){
		importIclefData("./rest/database.txt");
		readTrainingData("./analyser/data/dataFCTHFull.bin");
		createKnnIndex();
		extractorName = "fcth";
	}
	//importIclefData("./rest/database.txt");
	//readTrainingData("/home/amourao/code/lire/dataCEDDFull.bin");
	//createKnnIndex();
	//extractorName = "cedd";


}

MedicalSearchEngine::MedicalSearchEngine(){

	FactoryEndpoint::getInstance()->registerType("/medicalSearch",this);
	FactoryEndpoint::getInstance()->registerType("/imageClefImage",this);

}

MedicalSearchEngine::~MedicalSearchEngine(){}

void* MedicalSearchEngine::createType(string& type){
	//TODO
	std::cout << "New type requested: " << type << std::endl;



	if (type == "/medicalSearch"){
		if (instances.find(type) == instances.end())
			instances[type] =  new MedicalSearchEngine(type);

		return instances[type];
	} else if (type == "/imageClefImage")
		return new MedicalSearchEngine(type);


	std::cerr << "Error registering type from constructor (this should never happen)" << std::endl;
	return NULL;
}


void MedicalSearchEngine::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  	if(method != "GET"){
		resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		resp.send();
		return ;
	}

	if (type == "/medicalSearch"){
		resp.setContentType("text/html");

		if(queryStrings["format"] == "json")
			resp.setContentType("application/json");
		else if(queryStrings["format"] == "html")
			resp.setContentType("text/html");

		std::ostream& out = resp.send();

		std::string response("");

		if (type == "/medicalSearch")
			response = medicalSearch(queryStrings);

	//std::cout << response << std::endl;
		out << response;
		out.flush();
	} else if (type == "/imageClefImage"){

		std::string fileName = queryStrings["iri"];
		std::stringstream ss;
		ss << "/localstore/novameds/iclefmed12/fast.hevs.ch/imageclefmed/2012/figures/images/" << fileName << ".jpg";
		resp.sendFile(ss.str(),"image/jpeg");

	}

}

std::string MedicalSearchEngine::medicalSearch(map<string, string> parameters){
	FileDownloader fd;

	vector<string> filenames = fd.getFiles(parameters["images"]);
	string queryText = parameters["q"];
	string searchType = parameters["type"];
	string combFunction = parameters["comb"];

	std::vector<std::vector<SearchResult> > fullSearchResults;

	for (unsigned int j = 0; j < filenames.size(); j++)
		fullSearchResults.push_back(searchSingleImage(filenames[j],searchType));

	std::priority_queue<SearchResult,std::vector<SearchResult>,compareSearchResults> sortedList = CombSearchResult::combineResultsList(fullSearchResults, combFunction);

	if(parameters["format"] == "json")
		return resultListToJSON(queryText, filenames, combFunction, sortedList);
	else if(parameters["format"] == "html")
		return resultListToHTML(queryText, filenames, combFunction, sortedList, fullSearchResults);

	return resultListToHTML(queryText, filenames, combFunction, sortedList, fullSearchResults);
}

std::vector<SearchResult> MedicalSearchEngine::searchSingleImage(string filename, string searchType){
	vector<float>* features = getFeatures(filename);

		cv::Mat query(1,features->size(),CV_32F);
		for (unsigned int a = 0; a < features->size(); a++){
			query.at<float>(0,a) = features->at(a);
		}

		vector<int> indices (500);
		vector<float> dists (500);
		//cout << j++ << endl;
		flannIndex->knnSearch(query,indices,dists,500);

		std::vector<SearchResult> singleImageSearchResults;
		for (unsigned int h = 0; h < indices.size(); h++){
			int id = indices.at(h);

			string resultId = "case";

			if(searchType == "case")
				resultId = IRItoDOI[idToIRI[id]];
			else if (searchType == "image")
				resultId = idToIRI[id];

			singleImageSearchResults.push_back(SearchResult(resultId,1-(dists.at(h)/(features->size()*7)) /*TODO*/, (h+1),idToIRI[id], IRItoDOI[idToIRI[id]]));
		}
		return singleImageSearchResults;
}


void MedicalSearchEngine::indexFeature(vector<float> features, int id){

}

vector<float>* MedicalSearchEngine::getFeatures(string filename){
	FactoryAnalyser * f = FactoryAnalyser::getInstance();
	IAnalyser* extractor= (IAnalyser*)f->createType(extractorName);
	IDataModel* data = extractor->getFeatures(filename);
	return (vector<float>*) data->getValue();
}

void MedicalSearchEngine::importIclefData(string textFile){
ifstream file(textFile.c_str(), ifstream::in);
	string line, iri, doi,idStr;
	while (getline(file, line)) {
		stringstream liness(line);
		getline(liness, idStr, ';');
		getline(liness, iri, ';');
		getline(liness, doi);

		idStr.erase (std::remove (idStr.begin(), idStr.end(), ' '), idStr.end());

		idToIRI[atoi(idStr.c_str())] = iri;
		IRItoDOI[iri] = doi;
	}

}


void MedicalSearchEngine::readTrainingData(string baseFile) {
	/*
for(int j = 0; j < 24; j++){
	//Mat featuresRow;
	//Mat labelsRow;

	std::stringstream ss;
	if(j < 10)
		ss << baseFile << "dataSplit0" << j << "_histLBP.bin";
	else
		ss << baseFile << "dataSplit" << j << "_histLBP.bin";

	std::string filename = ss.str();
	MatrixTools::readBin(filename, features, labels);

}
*/
MatrixTools::readBin(baseFile, features, labels);
}

void MedicalSearchEngine::createKnnIndex() {

//string baseFileHist(argv[3]);
//string baseFileSegHist(argv[4]);


//void retrieveSimilar( cv::Mat query, int neighboursCount,vector<int>&  _indices, vector<float>& _dists)

/*
Mat featuresHist;
Mat labelsHist;

for(int j = 0; j < 4; j++){
	//Mat featuresRow;
	//Mat labelsRow;

	stringstream ss;
	if(j < 10)
		ss << baseFileHist << "xa0" << j << ".bin";
	else
		ss << baseFileHist << "xa" << j << ".bin";

	string filename = ss.str();
	MatrixTools::readBin(filename, featuresHist, labelsHist);


}

Mat featuresHistSplit;
Mat labelsHistSplit;


for(int j = 0; j < 24; j++){
	//Mat featuresRow;
	//Mat labelsRow;

	stringstream ss;
	if(j < 10)
		ss << baseFileSegHist << "dataSplit0" << j << "_seg.bin";
	else
		ss << baseFileSegHist << "dataSplit" << j << "_seg.bin";

	string filename = ss.str();
	MatrixTools::readBin(filename, featuresHistSplit, labelsHistSplit);


}
*/

//hconcat(features,featuresHist,newFeatures);
//hconcat(labels,labelsHist,newLabels);

	cv::flann::LinearIndexParams params = cv::flann::LinearIndexParams();



    cout << flannIndex << " " << features.cols << " " << features.rows << endl;
	if ( flannIndex != NULL)
		delete flannIndex;
	flannIndex = new cv::flann::Index();

	flannIndex->build(features,params);

//HistogramExtractor hist (16);
//GaborExtractor gabor(640,480,4,8);
//SegmentedHistogramExtractor segHist (16,1,1,true);

/*
map<string,double> scores;

int oldID = -1;
Mat src;
for (int k = 0; k < is.getImageCount(); k++) {
	if (!(src = is.nextImage()).empty()) {

		string info = is.getImageInfo();
		stringstream liness(info);

		string path, id1, id2, id3;

		getline(liness, path, ';');
		getline(liness, id1, ';');
		getline(liness, id2, ';');
		getline(liness, id3);

		int id1Int = atoi(id1.c_str());
		int id2Int = atoi(id2.c_str());
		int id3Int = atoi(id3.c_str());

		cv::Mat featuresRow1,featuresRow2,featuresRow,dst;

		if (src.channels() == 1) {
			cv::cvtColor(src, dst, CV_GRAY2BGR);
		} else
			dst = src;

		s->extractFeatures(dst, featuresRow1);
		s2->extractFeatures(dst, featuresRow2);
		hconcat(featuresRow1,featuresRow2,featuresRow);

		int neighboursCount = 10000;

		vector<int> indices;
		vector<float> dists;
		c.retrieveSimilar( featuresRow, neighboursCount,indices,dists);


		for (int i = 0; i < neighboursCount; i++){
			stringstream ss;
			ss << indices.at(i);

			double score = 1-(dists.at(i)/sqrt((s->getFeatureVectorSize()+s2->getFeatureVectorSize())));
			//double score = 1.0/(dists.at(i)+0.001);

			string iri;
			if (returnType == "case"){
				iri = IRItoDOI[idToIRI[indices.at(i)]]; //case based
			} else if (returnType == "image")
				iri = idToIRI[indices.at(i)]; //image based
			else
				cerr << "unknown type" << endl;

			if (scores.find(iri) == scores.end() || scores[iri] < score)
				scores[iri] = score;


			//cout << id1Int << " " << id2Int << "\t" << idToIRI[ss.str()] << "\t" << i+1 << "\t" << 1-(dists.at(i)/16/s->getFeatureVectorSize()*s2->getFeatureVectorSize()) << endl;
		}

		//getchar();

		if (id2Int == id3Int){

			priority_queue<scoreAndIRI, vector<scoreAndIRI>, CompareScoreAndIRI> orderedScores;


			std::map<string,double>::iterator iter = scores.begin();
			std::map<string,double>::iterator endIter = scores.end();
			for(; iter != endIter; ++iter){
				string iri = iter->first;
				double score = iter->second;
				orderedScores.push({score,iri});
			}

			for (int i = 0; i < 1000 && orderedScores.size() > 0; i++){
				scoreAndIRI a = orderedScores.top();
				orderedScores.pop();
				cout << id1Int << "\t1\t" << a.iri << "\t" << i+1 << "\t" << a.score << "\tFCT_SEGHISTLBP" << endl;
			}
			scores.clear();
		}
	}
}


return 0;
*/
}

string MedicalSearchEngine::resultListToJSON(string query, vector<string> images, string combFunction, std::priority_queue<SearchResult,std::vector<SearchResult>,compareSearchResults> sortedList){
	Json::Value root;
	Json::Value queryImages(Json::arrayValue);

	for(unsigned int i = 0; i < images.size(); i++)
		queryImages.append(images[i]);

	root["query"] = query;
	root["combFunction"] = combFunction;
	root["queryImages"] = queryImages;

	Json::Value resultArray(Json::arrayValue);

	for(unsigned int i = 0; i < sortedList.size(); i++){
		SearchResult sr = sortedList.top();
		sortedList.pop();
		Json::Value node;

		node["id"] = sr.getId();
		node["iri"] = sr.getIRI();
		node["doi"] = sr.getDOI();
		node["score"] = sr.getScore();
		node["rank"] = i+1;

		resultArray.append(node);
	}
	root["results"] = resultArray;
	stringstream ss;
	ss << root;
	return ss.str();
}

string MedicalSearchEngine::resultListToHTML(string queryText, vector<string> filenames, string combFunction, std::priority_queue<SearchResult,std::vector<SearchResult>,compareSearchResults> sortedList,std::vector<std::vector<SearchResult> > fullSearchResults){
	uint minimalResultSetSize = 9999999;
	stringstream altHtml;
	altHtml << "<html><body>" << endl;
	altHtml << "<h3>Query</h3>" << queryText << endl;

	altHtml << "<h3>Query images</h3>" << endl;

	for (unsigned int j = 0; j < filenames.size(); j++){
		altHtml << "<img src=\"file://" << filenames[j] << "\" height=\"200\">" << endl;
	}

	altHtml << "<h3>Combination Function</h3>" << combFunction << endl;

	altHtml << "<table><tr>" << endl;
	altHtml << "<th>Index</th>" << endl;
	for (unsigned int i = 0; i < fullSearchResults.size(); i++){
		//cout << fullSearchResults.at(i).size() << endl;
		if (minimalResultSetSize > fullSearchResults.at(i).size())
			minimalResultSetSize = fullSearchResults.at(i).size();
		altHtml << "<th>Results " << i << "</th>" << endl;
	}
	altHtml << "<th>Combined results</th>" << endl;
	altHtml << "</tr>" << endl;

	if (minimalResultSetSize > sortedList.size())
		minimalResultSetSize = sortedList.size();

	minimalResultSetSize = 50;
	//cout << sortedList.size() << endl;
	for (unsigned int j = 0; j < minimalResultSetSize; j++){
		altHtml << "<tr><td>" << j << "</td>" << endl;
		for (unsigned int i = 0; i < fullSearchResults.size(); i++){
			SearchResult sr = fullSearchResults.at(i).at(j);
			altHtml << "<td>" << endl;
			altHtml << "<img src=\"http://localhost:9090/imageClefImage?iri=" << sr.getIRI() << "\" width=\"200\">" << endl;
			altHtml << "<br />" << sr.getId() << "<br />"<< sr.getDOI() << "<br />" << sr.getIRI() << "<br />" << sr.getScore() << endl;
			altHtml << "</td>" << endl;
		}

		SearchResult sr = sortedList.top();
		sortedList.pop();
		altHtml << "<td>" << endl;
		altHtml << "<img src=\"http://localhost:9090/imageClefImage?iri=" << sr.getIRI() << "\" width=\"200\">" << endl;
		altHtml << "<br />" << sr.getId() << "<br />" << sr.getDOI() << "<br />" << sr.getIRI() << "<br />" << sr.getScore() << endl;
		altHtml << "</td>" << endl;

		altHtml << "</tr>" << endl;
	}
	altHtml << "</table>" << endl;

	altHtml << "</body></html>" << endl;

	return altHtml.str();
}


