#pragma once

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>


#include "RoiFeatureExtractor.h"
#include "../FactoryAnalyser.h"

using namespace std;
using namespace cv;

//const string FACE_DETECTION_CASCADE_PATH = "./analyser/data/HaarDetector/haarcascades/";
const string CASCADE_PATH = "./analyser/data/FaceDetection/haarcascades/";

class HaarDetector: public RoiFeatureExtractor
{
public:
	HaarDetector();
	HaarDetector(string& type);
	HaarDetector(string& type, map<string,string>& params);
    HaarDetector(string objectType, string cascadePath, double scaleChangeFactor, cv::Size minSize, cv::Size maxSize);
	~HaarDetector();

    void init(string objectType, string cascadePath, double scaleChangeFactor, cv::Size minSize, cv::Size maxSize);
    void* createType(string &typeId);
    void* createType(string &typeId, map<string,string>& params);

	void extractFeatures(Mat& source, map<string,region>& features);

	void detectObjects(Mat& image, vector<Mat>& objImages, vector<cv::Point>& locations, vector<Rect>& rois);

	void train(string trainFile);

	string test(string testFile);

	string crossValidate(string testFile);

	string getName();

private:

    string type;
    string objectType;

	double scaleChangeFactor;

	string cascadePath;
	cv::Size minSize;
	cv::Size maxSize;

	CascadeClassifier cascade;

};

