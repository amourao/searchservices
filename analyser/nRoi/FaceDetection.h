#pragma once

#ifndef PI
#define PI 3.14159265
#endif



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

//const string FACE_DETECTION_CASCADE_PATH = "./analyser/data/FaceDetection/haarcascades/";
const string FACE_DETECTION_CASCADE_PATH = "./analyser/data/FaceDetection/haarcascades/";

typedef struct {
	cv::Point eyeCenter;
	double eyeRadius;
} Eye;

class FaceDetection: public RoiFeatureExtractor
{
public:
	FaceDetection();
	FaceDetection(string& type);
	FaceDetection(string& type, map<string,string>& params);
    FaceDetection(string cascadePath, string nestedCascadePath, double scaleChangeFactor, double eyeScaleChangeFactor, cv::Size minSize, cv::Size maxSize, bool preProcess);
	~FaceDetection();

    void init(string cascadePath, string nestedCascadePath, double scaleChangeFactor, double eyeScaleChangeFactor, cv::Size minSize, cv::Size maxSize, bool preProcess);
    void* createType(string &typeId);
    void* createType(string &typeId, map<string,string>& params);

	void extractFeatures(Mat& source, map<string,region>& features);

	void detectFaces(Mat& image, vector<Mat>& faceImages, vector<cv::Point>& locations, vector<Rect>& faceRois);

	void train(string trainFile);

	string test(string testFile);

	string crossValidate(string testFile);

	string getName();

private:

    string type;

	bool preProcessFaceImage(Mat& image, Mat& faceImage, cv::Rect& roi);

	bool detectBestEyepair (Point& center, double faceRadius, vector<Eye>& leftEyes, vector<Eye>& rightEyes, Eye& bestLeftEye, Eye& bestRightEye);
	void rotateEye(cv::Point& center, cv::Point& eye, double angle);
	cv::Rect computeROI(cv::Point& center, cv::Point& eye1, cv::Point& eye2, int radius);
	void ajustROI (cv::Rect& roi, cv::Size s);

	double scaleChangeFactor;
	double eyeScaleChangeFactor;
	string cascadePath;
	string nestedCascadePath;
	cv::Size minSize;
	cv::Size maxSize;

	CascadeClassifier faceCascade;
	CascadeClassifier eyesCascade;

	bool preProcess;
};

