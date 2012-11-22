#pragma once

#ifndef PI
#define PI 3.14159265
#endif PI

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <iostream>


using namespace std;
using namespace cv;

typedef struct {
	cv::Point eyeCenter;
	double eyeRadius;
} Eye;

class FaceDetection
{
public:
	FaceDetection(string cascadePath, string nestedCascadePath, double scaleChangeFactor, double eyeScaleChangeFactor, cv::Size& minSize, cv::Size& maxSize);
	~FaceDetection();

	void detectFaces(Mat& image, vector<Mat>& faceImages, vector<cv::Point>& locations, vector<Rect>& faceRois, bool preProcess = true);

	

private:

	bool preProcessFaceImage(Mat& image, Mat& faceImage);

	bool detectBestEyepair (Point& center, double faceRadius, vector<Eye>& leftEyes, vector<Eye>& rightEyes, Eye& bestLeftEye, Eye& bestRightEye);
	void rotateEye(cv::Point& center, cv::Point& eye, double angle);
	cv::Rect computeROI(cv::Point& center, cv::Point& eye1, cv::Point& eye2, int radius);
	void FaceDetection::ajustROI (cv::Rect& roi, cv::Size& s);

	double scaleChangeFactor;
	double eyeScaleChangeFactor;
	string cascadePath;
	string nestedCascadePath;
	cv::Size minSize;
	cv::Size maxSize;

	CascadeClassifier faceCascade;
	CascadeClassifier eyesCascade;

};

