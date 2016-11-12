#include "SVMClassifier.h"


static SVMClassifier SVMClassifierFactory;

SVMClassifier::SVMClassifier(){
	FactoryClassifier::getInstance()->registerType("SVMClassifier",this);
}


SVMClassifier::SVMClassifier(string& _type){
	type = _type;
}

SVMClassifier::SVMClassifier(string& _type, std::map<string,string>& params){
	type = _type;
    if (params.count("trainFile") > 0)
        load(params["trainFile"]);
}


SVMClassifier::~SVMClassifier(){
}


void* SVMClassifier::createType(string& type){
	if (type == "SVMClassifier")
		return new SVMClassifier(type);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* SVMClassifier::createType(string& type, std::map<string,string>& params){
	return new SVMClassifier(type,params);
}


void SVMClassifier::train( cv::Mat trainData, cv::Mat trainLabels ){
	cv::Ptr<cv::ml::TrainData> data = cv::ml::TrainData::create(trainData,cv::ml::ROW_SAMPLE,trainLabels);
	//params.svm_type = CvSVM::C_SVC;
	//params.kernel_type = CvSVM::RBF;
	//params.gamma = 0.03375;
	//params.C = 62.5;
	//params.p = 1.1920928955078125e-007;
	svm = cv::ml::SVM::create();
	svm->trainAuto(data);

	//TODO
	//svm.train(trainData,trainLabels);
}

float SVMClassifier::classify( cv::Mat query){
	return svm->predict(query);
}

float SVMClassifier::getClassificationConfidence( cv::Mat query){
	return svm->predict(query);
}

void SVMClassifier::test( cv::Mat testData, cv::Mat testLabels ){
	//TODO
}
string SVMClassifier::getName(){
	return type;
}

bool SVMClassifier::save(string basePath){
	stringstream ss;
	ss << CLASSIFIER_BASE_SAVE_PATH << basePath << TRAINDATA_EXTENSION_SVM;

	if (svm != NULL)
		svm->save(ss.str());
	return true;



}

bool SVMClassifier::load(string basePath){
	stringstream ss;
	ss << CLASSIFIER_BASE_SAVE_PATH << basePath << TRAINDATA_EXTENSION_SVM;
    string path = ss.str();

	if (FileDownloader::fileExists(path)){
        svm = cv::ml::StatModel::load<cv::ml::SVM>(path);
        return true;
	} 
	return false;
}
