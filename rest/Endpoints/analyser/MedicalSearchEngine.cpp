#include "MedicalSearchEngine.h"

static MedicalSearchEngine medicalSearchEngineEndpointFactory;

MedicalSearchEngine::MedicalSearchEngine(string type){
	flannIndex = NULL;
	this->type = type;
	
	importIclefData("./rest/database.txt");
	createKnnIndex("/home/amourao/iclef2013Features/SegHistogramLBP/");
	extractorName = "SEG_LBP_HIST";
	
 
}

MedicalSearchEngine::MedicalSearchEngine(){

	FactoryEndpoint::getInstance()->registerType("/medicalSearch",this);

}

MedicalSearchEngine::~MedicalSearchEngine(){}

void* MedicalSearchEngine::createType(string& type){
	//TODO
	std::cout << "New type requested: " << type << std::endl;
	
	if (type == "/medicalSearch")
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

	//resp.setContentType("application/json");
	resp.setContentType("text/html");
	std::ostream& out = resp.send();

	std::string response("");

	if (type == "/medicalSearch")
		response = medicalSearch(queryStrings);

	//std::cout << response << std::endl;
	out << response;
	out.flush();


}

std::string MedicalSearchEngine::medicalSearch(map<string, string> parameters){
	
	stringstream altHtml;
	
	
	
	FileDownloader fd;
	
	vector<string> filenames = fd.getFiles(parameters["images"]);
	string queryText = parameters["q"];
	altHtml << "<html><body><h3>Query text<h3>" << queryText;
		
	Json::Value root;
	Json::Value nameArray(Json::arrayValue);
	
	
	altHtml << "<h3>Query images<h3>";
	for (unsigned int j = 0; j < filenames.size(); j++){
		
		altHtml << "<img src=\"file://" << filenames[j] << "\" height=\"200\">";
		nameArray.append(filenames[j]);
		vector<float>* features = getFeatures(filenames[j]);
		
		cv::Mat query(1,features->size(),CV_32F);
		for (unsigned int a = 0; a < features->size(); a++){
			query.at<float>(0,a) = features->at(a);
		}
		
		vector<int> indices (100);
		vector<float> dists (100);
		//cout << j++ << endl;
		flannIndex->knnSearch(query,indices,dists,100);
		Json::Value resultsArray(Json::arrayValue);
		altHtml << "<h3>Results<h3>";
		for (unsigned int h = 0; h < indices.size(); h++){
			
			Json::Value tag;
			int id = indices.at(h);
			tag["id"] = id;
			tag["doi"] = IRItoDOI[idToIRI[id]];
			tag["iri"] = idToIRI[id];
			resultsArray.append(tag);
			altHtml << "<h3>" << h << "<h3>" << id << " " << IRItoDOI[idToIRI[id]] << " " << idToIRI[id];
				
		
			altHtml << "<img src=\"file:///home/amourao/iclefmed12/fast.hevs.ch/imageclefmed/2012/figures/images/" << idToIRI[id] << ".jpg\" height=\"200\">";
			//cout << detectedEmotion << " " << dists.at(i)  << endl;
			//cout << query.colRange(0,5) << endl;
			//cout << trainData.row(imageId).colRange(0,5) << endl;
			//cout << norm(query,trainData.row(imageId)) << endl;
			//cout << imageId << endl;
		}
		Json::Value results;
		Json::Value featureArray(Json::arrayValue);
		
		
		for (unsigned int i = 0; i < features->size(); i++){
		
			featureArray.append(features->at(i));
			//cout << features->at(i) << " " ;
		}
	/*
	for(int i = 0; i < 8; i++)
	{
		Json::Value result;
		Json::Value tags;
		result["id"] = rand() % 25000;
		result["rank"] = rand() % 10;
		result["path"] = "/some/path/";
		result["title"] = "randomTitle";
		tags["0"] = "tag1";
		tags["1"] = "tag2";
		tags["2"] = "tag3";
		result["tags"] = tags;
		results[i] = result;
	}
	*/
		root[filenames[j]] = resultsArray;
	}
	root["query"] = queryText;
	root["imageUrls"] = nameArray;
	stringstream ss;
	ss << root;
	
	altHtml << "</body></html>";
	return altHtml.str();
	
}

void MedicalSearchEngine::indexFeature(vector<float> features, int id){
	
}

vector<float>* MedicalSearchEngine::getFeatures(string filename){
	
	FileDownloader fd;
			
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

void MedicalSearchEngine::createKnnIndex(string baseFile) {

//string baseFileHist(argv[3]);
//string baseFileSegHist(argv[4]);


//void retrieveSimilar( cv::Mat query, int neighboursCount,vector<int>&  _indices, vector<float>& _dists)
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
