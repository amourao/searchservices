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
	CvSVMParams params;
	//params.svm_type = CvSVM::C_SVC;
	//params.kernel_type = CvSVM::RBF;
	//params.gamma = 0.03375;
	//params.C = 62.5;
	//params.p = 1.1920928955078125e-007;
	svm = new CvSVM();
	svm->train_auto(trainData,trainLabels,cv::Mat(),cv::Mat(),params);

	//TODO
	//svm.train(trainData,trainLabels);
}

float SVMClassifier::classify( cv::Mat query){
	return svm->predict(query);
}

void SVMClassifier::test( cv::Mat testData, cv::Mat testLabels ){
	//TODO
}
string SVMClassifier::getName(){
	return "SVMClassifier";
}

bool SVMClassifier::save(string basePath){
	stringstream ss;
	ss << CLASSIFIER_BASE_SAVE_PATH << basePath << TRAINDATA_EXTENSION_SVM;

	if (svm != NULL)
		svm->save(ss.str().c_str());
	return true;



}

bool SVMClassifier::load(string basePath){
	stringstream ss;
	ss << CLASSIFIER_BASE_SAVE_PATH << basePath << TRAINDATA_EXTENSION_SVM;

	svm = new CvSVM();
	svm->load(ss.str().c_str());
	return true;
}
