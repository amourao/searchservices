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
	dst = Mat(src,true);
}

void MatrixTools::matToVectors(cv::Mat &src, vector<vector<float> >& dst){
	for(int i = 0; i < src.rows; i++){
		vector<float> tempRow;
		Mat row = src.row(i);
		MatrixTools::matToVector(row,tempRow);
		dst.push_back(tempRow);
	}
}

void MatrixTools::vectorsToMat(vector<vector<float> >&src, cv::Mat& dst){
	for(unsigned int i = 0; i < src.size(); i++){
		Mat tempRow;
		MatrixTools::vectorToMat(src.at(i),tempRow);
		dst.push_back(tempRow);
	}
}

//Atention, matrices must be in CV_32F format (single dimensional float matrix)
void MatrixTools::readBin(string& file, cv::Mat& features, cv::Mat& labels){

	string f = file;
 
 	float numSamples2 = 0;
 	float dimensions = 0;
 
	
 
	std::fstream ifs( f.c_str(), std::ios::in | std::ios::binary );
	

	ifs.read( (char*) &dimensions, sizeof(dimensions) );
	ifs.read( (char*) &numSamples2, sizeof(numSamples2) );

 
	for (int i = 0; i < numSamples2; i++){
		float label = 0;
		float id = 0;
		Mat featuresRow (1,dimensions,CV_32F);
		Mat labelsRow (1,2,CV_32F);
		
		ifs.read( (char*) &id, sizeof(id) );
		ifs.read( (char*) &label, sizeof(label) );

		
		labelsRow.at<float>(0,0) = label;
		labelsRow.at<float>(0,1) = id;
					
		for (int j = 0; j < dimensions; j++){
			float feature = 0;
			ifs.read( (char*) &feature, sizeof(feature) );
			featuresRow.at<float>(0,j) = feature;

		}
		labels.push_back(labelsRow);
		features.push_back(featuresRow);
		
	}
	ifs.close();
}

//Atention, matrices must be in CV_32F format (single dimensional float matrix)
void MatrixTools::writeBin(string& filename, cv::Mat& features, cv::Mat& labels){
	std::fstream binFile;
	binFile.open(filename.c_str(), std::ios::out | std::ios::binary);
	
	float dims = features.cols;
	binFile.write((const char*) &dims, sizeof(float));
	float nSamples = features.rows;
	binFile.write((const char*) &nSamples, sizeof(float));
	
	//std::cout << features.cols << " " << features.rows << endl;
	
	for (int s = 0; s < nSamples; s++) {


		float label;
		float id;
		
		if (labels.cols > 1){
			label = (int) labels.at<float>(nSamples,0);
			id = (int) labels.at<float>(nSamples,1);
		}
		else {
			label = (int) labels.at<float>(nSamples,0);
			id = label;
		}
		
		binFile.write((const char*) &id, sizeof(float));
		binFile.write((const char*) &label, sizeof(float));

		for (int i = 0; i < dims; i++) {
			float value = features.at<float>(s,i);
			binFile.write((const char*) &value, sizeof(float));
		}
		binFile.flush();
	}

	binFile.close();
	
}
