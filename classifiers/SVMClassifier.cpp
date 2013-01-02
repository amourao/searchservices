#include "SVMClassifier.h"


SVMClassifier::SVMClassifier(){
}


SVMClassifier::~SVMClassifier(){
}

void SVMClassifier::train( Mat trainData, Mat trainLabels ){
	CvSVMParams params;
	params.svm_type = CvSVM::C_SVC;
	params.kernel_type = CvSVM::RBF;
	//params.gamma = 0.03375;
	//params.C = 62.5;
	//params.p = 1.1920928955078125e-007;
	svm.train_auto(trainData,trainLabels,Mat(),Mat(),params);
}

float SVMClassifier::classify( Mat query){
	return svm.predict(query);
}

void SVMClassifier::test( Mat testData, Mat testLabels ){
	//TODO
}
