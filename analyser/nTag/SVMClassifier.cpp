#include "SVMClassifier.h"


SVMClassifier::SVMClassifier(){
}


SVMClassifier::~SVMClassifier(){
}

void SVMClassifier::train( cv::Mat trainData, cv::Mat trainLabels ){
	CvSVMParams params;
	//params.svm_type = CvSVM::C_SVC;
	//params.kernel_type = CvSVM::RBF;
	//params.gamma = 0.03375;
	//params.C = 62.5;
	//params.p = 1.1920928955078125e-007;
	svm.train_auto(trainData,trainLabels,cv::Mat(),cv::Mat(),params);
	
	//TODO
	//svm.train(trainData,trainLabels);
}

float SVMClassifier::classify( cv::Mat query){
	return svm.predict(query);
}

void SVMClassifier::test( cv::Mat testData, cv::Mat testLabels ){
	//TODO
}
string SVMClassifier::getName(){
	return "SVM";
}

bool SVMClassifier::save(string basePath){
	//TODO do the function
	return false;
}

bool SVMClassifier::load(string basePath){
	//TODO do the function
	return false;
}