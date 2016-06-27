#pragma once

#include <armadillo>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <memory>


#include "IBinImporter.h"

//#define BIN_SIGNATURE_HEX 4e534e53
#define BIN_SIGNATURE_INT 1314082387

#define BIN_VERSION 2
#define NBIN_VERSION 3

using namespace cv;
using namespace std;

class MatrixTools: public IBinImporter{
public:

MatrixTools();
~MatrixTools();

void readBin(std::string file, int numberOfRows, cv::Mat& features, long long offsetInRows = 0);
void readBin(std::string filenamep, int numberOfRows, arma::fmat& features, long long offsetInRows = 0);
void readTags(std::string file, int numberOfRows, cv::Mat& tags);

static void fmatToMat(arma::fmat &src, cv::Mat& dst);

static void matToFMat(cv::Mat &src, arma::fmat& dst, bool copyData=false);

static void matToVector(cv::Mat &src, vector<float>& dst);

static void matToVectors(cv::Mat &src, vector<vector<float> >& dst);

static void vectorToMat(vector<float>&src, cv::Mat& dst);

static void vectorsToMat(vector<vector<float> >&src, cv::Mat& dst);


//Attention, matrices must be in CV_32F format (single dimensional float matrix)
static void readBin(string file, cv::Mat& features, cv::Mat& labels);
//Attention, matrices must be in CV_32F format (single dimensional float matrix)
static void writeBin(string file, cv::Mat& features, cv::Mat& labels);

//Attention, matrices must be in CV_32F format (single dimensional float matrix)
static void readBinV1(string file, cv::Mat& features, cv::Mat& labels);
//Attention, matrices must be in CV_32F format (single dimensional float matrix)
static void writeBinV1(string file, cv::Mat& features, cv::Mat& labels);


static void readBinV2(string file, cv::Mat& features, cv::Mat& labels);
static void writeBinV2(string file, cv::Mat& features, cv::Mat& labels, bool append = false);
static void readBinV2(string file, vector<cv::Mat>& features, cv::Mat& labels);
static void writeBinV2(string file, vector<cv::Mat>& features, cv::Mat& labels, bool append = false);


static void readBinV3(string file, vector<cv::Mat>& features, cv::Mat& labels);
static void writeBinV3(string file, vector<cv::Mat>& features, cv::Mat& labels, bool append = false);
static void readBinV3(string file, vector<cv::Mat>& features, vector<vector<cv::KeyPoint> >& keypoints, cv::Mat& labels);
static void writeBinV3(string file, cv::Mat& features, vector<cv::KeyPoint>& keypoints, cv::Mat& labels, bool append = false);

static void keypointToMat(cv::KeyPoint& p, cv::Mat& m);
static void matToKeypoint(cv::Mat& m, cv::KeyPoint& p);

static void keypointsToMats(std::vector<cv::KeyPoint>& p,cv::Mat& m);
static void matToKeypoints(cv::Mat& m, std::vector<cv::KeyPoint>& p);

static void getRandomSample(vector<cv::Mat>& mList, int nRows, vector<cv::Mat>& sampleList);
static void getRandomSample(vector<arma::fmat>& mList, int nRows, vector<arma::fmat>& sampleList);


template <typename T, typename Compare>
static std::vector<std::size_t> sortPermutation(
    const std::vector<T>& vec,
    Compare& compare){
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
        [&](std::size_t i, std::size_t j){ return vec[i] < vec[j]; });
    return p;
}

template <typename T>
static std::vector<T> applyPermutation(
    const std::vector<T>& vec,
    const std::vector<std::size_t>& p,
    uint n){
    n = std::min(n,(uint)p.size());
    std::vector<T> sorted_vec(n);
    std::transform(p.begin(), p.begin()+n, sorted_vec.begin(),
        [&](std::size_t i){ return vec[i]; });
    return sorted_vec;
}

/*
template <typename T, typename Compare>
static std::vector<T> kWayMerge(
    const std::vector<T>& vec,
    Compare& compare){


}
*/
};
