#include "VWBasicClassifier.h"


VWBasicClassifier::VWBasicClassifier(){
	
}


VWBasicClassifier::~VWBasicClassifier(){
	VW::finish(*vwModel);
}

void VWBasicClassifier::train( cv::Mat trainData, cv::Mat trainLabels ){
	
	int numberOfClasses = 0;
	
	std::set<int> s;
	
	for(int i = 0; i < trainLabels.rows; i++){
		float value = trainLabels.at<float>(i,0);
		if (s.find(value) == s.end())
			s.insert(value);
	}
	numberOfClasses = s.size();
	
	stringstream ss;
	ss << "--oaa " << numberOfClasses;
	string params = ss.str();
			
	vwModel = VW::initialize(params);
	
	ezexample ex(vwModel, false);
	
	importToVowpalFormat(ex,trainData,trainLabels);		
}

float VWBasicClassifier::classify( cv::Mat query){
	ezexample ex(vwModel, false);
	return importToVowpalFormatTest(ex, query);
}

void VWBasicClassifier::test( cv::Mat testData, cv::Mat testLabels ){
	//TODO
}
string VWBasicClassifier::getName(){
	return "VWBasicClassifier";
}

void VWBasicClassifier::importToVowpalFormat(ezexample& ex, cv::Mat trainData, cv::Mat trainLabels){
	
	for(int i = 0; i < trainData.rows; i++){
	
		for(int j = 0; j < trainData.cols; j++)
			ex(vw_namespace('a'))(j+1)(trainData.at<float>(i,j));
				
		stringstream ss;
		
		ss << ((int) trainLabels.at<float>(i,0))+1 << " 1.0 " << i;
		
		ex.set_label(ss.str());
		ex.train();
	}
	ex.finish();
}

float VWBasicClassifier::importToVowpalFormatTest(ezexample& ex, cv::Mat testData){
	
	for(int i = 0; i < testData.rows; i++){
	
		for(int j = 0; j < testData.cols; j++)
			ex(vw_namespace('a'))(j+1)(testData.at<float>(i,j));
				
		stringstream ss;
		
		ss << " 1.0 ";
		
		ex.set_label(ss.str());
		ex.train();
	}
	return (float)ex.predict()-1;
}
