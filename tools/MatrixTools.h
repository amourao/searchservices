#pragma once

#include <armadillo>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <vector>

using namespace cv;
using namespace std;

class MatrixTools
{
public:
	
static void fmatToMat(arma::fmat &src, cv::Mat& dst);

static void matToFMat(cv::Mat &src, arma::fmat& dst);

static void matToVector(cv::Mat &src, vector<float>& dst);

static void vectorToMat(vector<float>&src, cv::Mat& dst);

};
