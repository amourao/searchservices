#pragma once

#include <armadillo>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>
#include <fstream>

#include "IBinImporter.h"

//#define BIN_SIGNATURE_HEX 4e534e53
#define BIN_SIGNATURE_INT 1314082387

#define BIN_VERSION 2

using namespace cv;
using namespace std;

class MatrixTools: public IBinImporter{
public:

MatrixTools();
~MatrixTools();

void readBin(std::string file, int numberOfRows, cv::Mat& features, long long offsetInRows = 0);
void readTags(std::string file, int numberOfRows, cv::Mat& tags);

static void fmatToMat(arma::fmat &src, cv::Mat& dst);

static void matToFMat(cv::Mat &src, arma::fmat& dst);

static void matToVector(cv::Mat &src, vector<float>& dst);

static void matToVectors(cv::Mat &src, vector<vector<float> >& dst);

static void vectorToMat(vector<float>&src, cv::Mat& dst);

static void vectorsToMat(vector<vector<float> >&src, cv::Mat& dst);


//Atention, matrices must be in CV_32F format (single dimensional float matrix)
static void readBin(string& file, cv::Mat& features, cv::Mat& labels);

//Atention, matrices must be in CV_32F format (single dimensional float matrix)
static void writeBin(string& file, cv::Mat& features, cv::Mat& labels);

//Atention, matrices must be in CV_32F format (single dimensional float matrix)
static void readBinV1(string& file, cv::Mat& features, cv::Mat& labels);

//Atention, matrices must be in CV_32F format (single dimensional float matrix)
static void writeBinV1(string& file, cv::Mat& features, cv::Mat& labels);


static void readBinV2(string& file, cv::Mat& features, cv::Mat& labels);

static void writeBinV2(string& file, cv::Mat& features, cv::Mat& labels);

static void readBinV2(string& file, vector<cv::Mat>& features, cv::Mat& labels);

static void writeBinV2(string& file, vector<cv::Mat>& features, cv::Mat& labels);

};
