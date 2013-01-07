#include "MatrixTools.h"

void MatrixTools::fmatToMat(arma::fmat &src, cv::Mat& dst){	
	dst.create(src.n_rows,src.n_cols,CV_32F);
	for (unsigned int i = 0; i < src.n_rows; i++)
		for (unsigned int j = 0; j < src.n_cols; j++)
			dst.at<float>(i,j) = src(i,j);
}

void MatrixTools::matToFMat(cv::Mat &src, arma::fmat& dst){	
	dst = arma::fmat(src.rows, src.cols);
	//cout << src.rows << " " << src.cols << endl;
	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++)
			dst(i,j) = src.at<float>(i,j);
}

void MatrixTools::matToVector(cv::Mat &src, vector<float>& dst){	
	float* array = (float*)src.data;
	dst = vector<float>(array, array + sizeof(float)*src.cols*src.rows);
}

void MatrixTools::vectorToMat(vector<float>&src, cv::Mat& dst){	
	for (unsigned int i = 0; i < src.size(); i++)
		dst.at<float>(1,i) = src.at(i);
}

