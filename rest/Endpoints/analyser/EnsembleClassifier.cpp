#include "EnsembleClassifier.h"

static EnsembleClassifier EnsembleClassifierEndpointFactory;

EnsembleClassifier::EnsembleClassifier(string type){
  this->type = type;
}

EnsembleClassifier::EnsembleClassifier(){

	FactoryEndpoint::getInstance()->registerType("/ensemble",this);

}

EnsembleClassifier::~EnsembleClassifier()
{}

void* EnsembleClassifier::createType(string& type){
	//TODO
	std::cout << "New type requested: " << type << std::endl;

	if (type == "/ensemble")
		return new EnsembleClassifier(type);

	std::cerr << "Error registering type from constructor (this should never happen)" << std::endl;
	return NULL;
}


void EnsembleClassifier::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  	if(method != "GET"){
		resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		resp.send();
		return ;
	}

	resp.setContentType("text/plain");
	std::ostream& out = resp.send();

	std::string response("");

	if (type == "/ensemble")
		response = getFeatures(queryStrings);

	//std::cout << response << std::endl;
	out << response;
	out.flush();


}

void EnsembleClassifier::readData(string filename, string extractorName){

	unsigned found = filename.find_last_of("/\\");

	string path = filename.substr(0,found);
	TextFileSource tfs(filename,path);

	FactoryAnalyser * f = FactoryAnalyser::getInstance();
	IAnalyser* extractor= (IAnalyser*)f->createType(extractorName);
	FeatureExtractor* fextractor = (FeatureExtractor*) extractor;

	Mat image;
	while(!(image = tfs.nextImage()).empty()){

		Mat featureVector;
		fextractor->extractFeatures(image, featureVector);
		features.push_back(featureVector);

		string info = tfs.getImageInfo();
		string id;
		string emo;
		string path;


		stringstream liness(info);
		getline(liness, path, ';');
		getline(liness, id, ';');
		getline(liness, emo, '\n');


		Mat label(1,3,CV_32F);
		label.at<float>(0,0) = atoi(emo.c_str());
		label.at<float>(0,1) = atoi(id.c_str());
		label.at<float>(0,2) = atoi(path.c_str());//DANGER, it relies on having the name of the file starting with a number
		labels.push_back(label);

	}
	//cout << labels;
}

string EnsembleClassifier::getFeatures(map<string, string > parameters){

	string filename = parameters["url"];
	string extractorName = parameters["extractor"];

	readData(filename,extractorName);


	stringstream ss5;




	//cout << sadsa << endl;
	ss5 << crossValidateAll(2,extractorName);

	return ss5.str();
}


string EnsembleClassifier::crossValidateAll(int numberOfDivisions, string extractorName){
	stringstream ss;

	double* min = new double();
	double* max = new double();

	int* minInd  = new int();
	int* maxInd  = new int();

	//cout << labels << endl;
	cv::minMaxIdx(labels.col(0), min, max, minInd, maxInd, Mat());

	int numberOfClasses = (*max)-(*min)+1;

	delete min;
	delete max;
	delete minInd;
	delete maxInd;

	//ss << "Validate all" << endl << endl;

	vector<int> correctGuesses(numberOfClasses,0);
	vector<int> falsePositives(numberOfClasses,0);

	Mat confusionMatrix= Mat::zeros(numberOfClasses,numberOfClasses,CV_32F);

	for(int j = 0; j < numberOfDivisions; j++){

		Mat newTrainData, newTrainLabels, testData, testLabels;
		divideByClass(features,labels,numberOfDivisions,j,newTrainData,newTrainLabels,testData,testLabels);

		//ss << newTrainData.rows << " " << newTrainData.cols << " " << testData.rows << " " << testData.cols << endl;
		//ss << newTrainLabels.rows << " " << newTrainLabels.cols << " " << testLabels.rows << " " << testLabels.cols << endl;
		index(newTrainData);
		ss << test(newTrainData,newTrainLabels,testData,testLabels,extractorName);
	}
	//cout << ss.str();
	return ss.str();
}


void EnsembleClassifier::divideByClass(Mat trainData, Mat trainLabels, double numberOfDivisions, int currentDivision,Mat& newTrainData,Mat& newTrainLabels, Mat& testData, Mat& testLabels){


	double ratio = 1.0/numberOfDivisions;
	double min, max;
	int minInd, maxInd;
	cv::minMaxIdx(trainLabels, &min, &max, &minInd, &maxInd, Mat());

	int numberOfClasses = max-min+1;
	vector<Mat> divisionByClass(numberOfClasses);
	vector<Mat> divisionByClassID(numberOfClasses);
	for(int j = 0; j < trainLabels.rows; j++){
		float label = trainLabels.at<float>(j,0);
		divisionByClass.at(label).push_back(trainData.row(j));
		divisionByClassID.at(label).push_back(trainLabels.row(j));
	}

	for(int i = 0; i < numberOfClasses; i++){

		Mat data = divisionByClass.at(i);
		Mat labels = divisionByClassID.at(i);
		if(!data.empty()){
			//TODO check if division is correct
			int divisionSize = ratio * data.rows;


			//cout << endl << "train starts at 0 and ends at " << (currentDivision)*divisionSize << endl;
			for(int j = 0; j < (currentDivision)*divisionSize; j++){
				newTrainData.push_back(data.row(j));
				newTrainLabels.push_back(labels.row(j));
			}

			//cout << "test starts at " <<  currentDivision*divisionSize << " and ends at " << ((currentDivision+1)*divisionSize) << " or " << data.rows << endl;
			for(int j = currentDivision*divisionSize; j < ((currentDivision+1)*divisionSize) && (j < data.rows); j++){
				testData.push_back(data.row(j));
				testLabels.push_back(labels.row(j));
			}

			//cout << "train starts at " <<  (currentDivision+1)*divisionSize << " and ends at " << data.rows << endl;
			for(int j = (currentDivision+1)*divisionSize; j < data.rows; j++){
				newTrainData.push_back(data.row(j));
				newTrainLabels.push_back(labels.row(j));
			}
			//cout << "train: " << newTrainData.rows << " test: " << testData.rows << endl;
		}

	}

}

void EnsembleClassifier::index(const Mat& trainData){
	cv::flann::LinearIndexParams params = cv::flann::LinearIndexParams();
	flannIndex = new cv::flann::Index();
	flannIndex->build(trainData,params);
}

string EnsembleClassifier::test(const Mat& trainData, const Mat& trainLabels, const Mat& testData, const Mat& testLabels,string extractorName){

	stringstream ss;

	FactoryAnalyser * f = FactoryAnalyser::getInstance();
	IAnalyser* extractor= (IAnalyser*)f->createType(extractorName);
	FeatureExtractor* fextractor = (FeatureExtractor*) extractor;

	for (int row = 0; row < testData.rows; row++){
		uint rankSize = 100;
		vector<int> indices (rankSize);
		vector<float> dists (rankSize);
		flannIndex->knnSearch(testData.row(row),indices,dists,rankSize);

		for (unsigned int h = 0; h < rankSize; h++){
			int id = indices.at(h);
			float dist = 1-(dists.at(h)/(fextractor->getFeatureVectorSize()*7));

			ss << (int)testLabels.at<float>(row,2) << "\t1\t" << (int)trainLabels.at<float>(id,2) << "\t" << h+1 << "\t" << dist << "\t" << extractorName << endl;
		}
	}
	return ss.str();


	/*
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

			singleImageSearchResults.push_back(SearchResult(resultId,1-(dists.at(h)/(features->size()*7)) , (h+1),idToIRI[id], IRItoDOI[idToIRI[id]]));
		}
		return singleImageSearchResults;

		*/
}
