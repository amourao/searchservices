#pragma once

#include <fftw3.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>

#include "../tools/IlluminationCorrectionFilter.h"
#include "FeatureExtractor.h"
#include "../tools/ImageFilter.h"
#include "../FactoryAnalyser.h"
#include "../../commons/StringTools.h"
#ifndef PI
#define PI 3.14159265359
#endif

using namespace cv;
using namespace std;


class GaborExtractor: public  FeatureExtractor, public ImageFilter
{
public:

	GaborExtractor(int imageW, int imageH, int nScales, int nOrientations, vector<cv::Rect> rectangles = vector<cv::Rect>(),int minWaveLength = 3,int mult = 2, double sigmaOnf = 0.65, double dThetaOnSigma = 1.5);

	GaborExtractor();
	GaborExtractor(string& type);
	GaborExtractor(string& type, map<string, string>& params);
	~GaborExtractor();

    void* createType(string &typeId);
	void* createType(string &typeId, map<string, string>& params);

    void init(int imageW, int imageH, int nScales, int nOrientations, vector<cv::Rect> rectangles,int minWaveLength,int mult , double sigmaOnf, double dThetaOnSigma);
	void applyFilter(Mat& src, Mat& dst);

	void extractFeatures(Mat& src, Mat& dst);

	int getFeatureVectorSize();

	string getName();

private:

    string type;
	int imageW;
	int imageH;
	int nScales;
	int nOrientations;
	int minWaveLength;
	int mult;
	double sigmaOnf;
	double dThetaOnSigma;
	vector<Mat> filters;

	vector<cv::Rect> rectangleRois;

	Mat illuminationFilterDarkFirst;
	Mat illuminationFilterBrightFirst;
	cv::Rect illumBaseRight;
	cv::Rect illumBaseLeft;
	IlluminationCorrectionFilter illumFilter;

	void buildFilters();
	void meshGrid(double minX, double maxX, double minY, double maxY, int rows, int cols, Mat& x, Mat& y);
	void cvShiftDFT(CvArr * src_arr, CvArr * dst_arr );
	fftw_complex* newFFTW(Mat image);
	Mat newIFFTW(fftw_complex* image, int width, int height);
	void preProcess(Mat& src, Mat& dst);

};

