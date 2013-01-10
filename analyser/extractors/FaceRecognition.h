#pragma once


#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

typedef struct {
	int personId;
	string personName;
	cv::Point center;
	int radius;
	vector<cv::Point> trajectory;
	vector<Mat> imgList;
	bool isInTrainingData;
	bool isAmbiguous;


} trackedPerson;

class FaceRecognition{

public:
	FaceRecognition(string trainDataFile);
	FaceRecognition(string xmlFile, string indexFile);
	~FaceRecognition();


	void train(string& trainDataFile);
	void test(string& testDataFile);
	
	bool load(string& trainDataFile, string& storedFlannData);
	void save(string& storedtrainData, string& storedFlannData);
	
	void recognizePeople(vector<Mat>& unknown,vector<cv::Point>& center, vector<trackedPerson>& output);
};

