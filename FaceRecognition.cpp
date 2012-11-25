#include "FaceRecognition.h"


FaceRecognition::FaceRecognition(string trainDataFile){
}
FaceRecognition::FaceRecognition(string xmlFile, string indexFile){}
FaceRecognition::~FaceRecognition(){}


void FaceRecognition::train(string& trainDataFile){}
void FaceRecognition::test(string& testDataFile){}
	
bool FaceRecognition::load(string& trainDataFile, string& storedFlannData){}
void FaceRecognition::save(string& storedtrainData, string& storedFlannData){}
	
void FaceRecognition::recognizePeople(vector<Mat>& unknown,vector<cv::Point>& center, vector<trackedPerson>& output){}
