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
		
		Mat label(1,2,CV_32F);
		label.at<float>(0,0) = atoi(emo.c_str());
		label.at<float>(0,1) = atoi(id.c_str());
		labels.push_back(label);
	}
}

string EnsembleClassifier::getFeatures(map<string, string > parameters){
		
	string filename = parameters["url"];
	string extractorName = parameters["extractor"];
	
	readData(filename,extractorName);
	
	
	
	stringstream ss5;
	
	ss5 << features.rows << " " << features.cols << " " << labels.rows << " " << labels.cols;
	
	string sadsa = ss5.str();
	
	//cout << sadsa << endl;
	crossValidateAll(2);
	
	return sadsa;
}


string EnsembleClassifier::crossValidateAll(int numberOfDivisions){
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

	ss << "Validate all" << endl << endl;
	
	vector<int> correctGuesses(numberOfClasses,0);
	vector<int> falsePositives(numberOfClasses,0);
	
	Mat confusionMatrix= Mat::zeros(numberOfClasses,numberOfClasses,CV_32F);		
		
	for(int j = 0; j < numberOfDivisions; j++){
		
		Mat newTrainData, newTrainLabels, testData, testLabels;
		divideByClass(features,labels,numberOfDivisions,j,newTrainData,newTrainLabels,testData,testLabels);
		
		
		//ss << test(newTrainData,newTrainLabels,testData,testLabels);
	}
	//cout << ss.str() << endl;
	return ss.str();
}


void EnsembleClassifier::divideByClass(Mat trainData, Mat trainLabels, double numberOfDivisions, int currentDivision,Mat& newTrainData,Mat& newTrainLabels, Mat& testData, Mat& testLabels){
		
		
	double ratio = 1.0/numberOfDivisions;
	double min, max;
	int minInd, maxInd;
	cv::minMaxIdx(trainLabels, &min, &max, &minInd, &maxInd, Mat());
	
	int numberOfClasses = max-min+1;
	vector<Mat> divisionByClass(numberOfClasses);
	for(int j = 0; j < trainLabels.rows; j++){
		float label = trainLabels.at<float>(j,0);
		divisionByClass.at(label).push_back(trainData.row(j));
	}
	
	for(int i = 0; i < numberOfClasses; i++){
		
		Mat data = divisionByClass.at(i);
		if(!data.empty()){
			//TODO check if division is correct
			int divisionSize = ratio * data.rows;
		
			Mat label(1,1,CV_32F);
			label.at<float>(0,0) = i;
			
			//cout << endl << "train starts at 0 and ends at " << (currentDivision)*divisionSize << endl;
			for(int j = 0; j < (currentDivision)*divisionSize; j++){
				newTrainData.push_back(data.row(j));
				newTrainLabels.push_back(label);
			}
		
			//cout << "test starts at " <<  currentDivision*divisionSize << " and ends at " << ((currentDivision+1)*divisionSize) << " or " << data.rows << endl;
			for(int j = currentDivision*divisionSize; j < ((currentDivision+1)*divisionSize) && (j < data.rows); j++){
				testData.push_back(data.row(j));
				testLabels.push_back(label);
			}
		
			//cout << "train starts at " <<  (currentDivision+1)*divisionSize << " and ends at " << data.rows << endl;
			for(int j = (currentDivision+1)*divisionSize; j < data.rows; j++){
				newTrainData.push_back(data.row(j));
				newTrainLabels.push_back(label);
			}
			//cout << "train: " << newTrainData.rows << " test: " << testData.rows << endl;
		}
		
	}
	
}


string EnsembleClassifier::test(Mat& trainData, Mat& trainLabels, Mat& testData, Mat& testLabels){

}
