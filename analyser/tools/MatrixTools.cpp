#include "MatrixTools.h"

MatrixTools::MatrixTools(){}
MatrixTools::~MatrixTools(){}


void MatrixTools::readBin(std::string file, int numberOfRows, cv::Mat& features, long long offsetInRows){
    Mat tmpFeatures;
    Mat labels;
    MatrixTools::readBin(file, tmpFeatures, labels);
    features = tmpFeatures.rowRange(offsetInRows,numberOfRows+offsetInRows);
}

void MatrixTools::readTags(std::string file, int numberOfRows, cv::Mat& tags){}

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
	dst = vector<float>(array, array + src.cols*src.rows);
}

void MatrixTools::vectorToMat(vector<float>&src, cv::Mat& dst){
	dst = Mat(src,true);
	transpose(dst,dst);
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

void MatrixTools::readBin(string& file, cv::Mat& features, cv::Mat& labels){
	std::fstream ifs( file.c_str(), std::ios::in | std::ios::binary );
	int signature;
	ifs.read( (char*) &signature, sizeof(signature));
	if (signature == BIN_SIGNATURE_INT){
		ifs.close();
		MatrixTools::readBinV2(file, features, labels);
	} else {
		ifs.close();
		MatrixTools::readBinV1(file, features, labels);
	}
}

//Atention, matrices must be in CV_32F format (single dimensional float matrix)
void MatrixTools::writeBin(string& filename, cv::Mat& features, cv::Mat& labels){
	MatrixTools::writeBinV1(filename, features, labels);
}

//Atention, matrices must be in CV_32F format (single dimensional float matrix)
void MatrixTools::readBinV1(string& file, cv::Mat& features, cv::Mat& labels){

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


		labelsRow.at<float>(0,1) = label;
		labelsRow.at<float>(0,0) = id;

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
void MatrixTools::writeBinV1(string& filename, cv::Mat& features, cv::Mat& labels){
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


void MatrixTools::readBinV2(string& file, cv::Mat& features, cv::Mat& labels){
	vector<cv::Mat> featuresNew;

	MatrixTools::readBinV2(file, featuresNew, labels);
	for (uint i = 0; i < featuresNew.size(); i++){
		Mat row = featuresNew.at(i);
		features.push_back(row);
	}
}

void MatrixTools::writeBinV2(string& file, cv::Mat& features, cv::Mat& labels, bool append ){
	vector<cv::Mat> featuresNew;

	for (int i = 0; i < features.rows; i++){
		Mat row = features.row(i);
		featuresNew.push_back(row);
	}
	MatrixTools::writeBinV2(file, featuresNew, labels, append);
}

//Atention, matrices must be in CV_32F format (single dimensional float matrix)
void MatrixTools::readBinV2(string& file, vector<cv::Mat>& features, cv::Mat& labels){
	string f = file;

 	int signature,binV,nSamples,dimsX,dimsY,dimsZ;

	std::fstream ifs( f.c_str(), std::ios::in | std::ios::binary );


	ifs.read( (char*) &signature, sizeof(signature));
	if (signature != BIN_SIGNATURE_INT){
		cerr << "Read error: Wrong signature" << endl;
		return;
	}

	ifs.read( (char*) &binV, sizeof(binV));

	if (binV != BIN_VERSION){
		cerr << "Read error: Wrong version" << endl;
		return;
	}

	ifs.read( (char*) &nSamples, sizeof(nSamples));
	ifs.read( (char*) &dimsX, sizeof(dimsX));
	ifs.read( (char*) &dimsY, sizeof(dimsY));
	ifs.read( (char*) &dimsZ, sizeof(dimsZ));

	for (int s = 0; s < nSamples; s++) {
		float value;
		Mat tmpLabels;


		for (int i = 0; i < dimsZ; i++) {
			ifs.read( (char*) &value, sizeof(value));
			tmpLabels.push_back(value);
		}
		if (!tmpLabels.empty()){
			transpose(tmpLabels, tmpLabels);
			labels.push_back(tmpLabels);
		}
		Mat featureMatrix (dimsY,dimsX,CV_32F);
		for (int x = 0; x < dimsX; x++) {
			for (int y = 0; y < dimsY; y++) {
				ifs.read( (char*) &value, sizeof(value));
				featureMatrix.at<float>(y,x) = value;
			}
		}
		features.push_back(featureMatrix);
	}
}


//Atention, matrices must be in CV_32F format (single dimensional float matrix)
void MatrixTools::readBinV3(string& file, vector<cv::Mat>& features, cv::Mat& labels){
	string f = file;

 	int signature,binV,nSamples,dimsX,dimsY,dimsZ;

	std::fstream ifs( f.c_str(), std::ios::in | std::ios::binary );


	ifs.read( (char*) &signature, sizeof(signature));
	if (signature != BIN_SIGNATURE_INT){
		cerr << "Read error: Wrong signature" << endl;
		return;
	}

	ifs.read( (char*) &binV, sizeof(binV));

	if (binV != NBIN_VERSION){
		cerr << "Read error: Wrong version" << endl;
		return;
	}

	ifs.read( (char*) &nSamples, sizeof(nSamples));

	for (int s = 0; s < nSamples; s++) {

        ifs.read( (char*) &dimsX, sizeof(dimsX));
        ifs.read( (char*) &dimsY, sizeof(dimsY));
        ifs.read( (char*) &dimsZ, sizeof(dimsZ));

		float value;
		Mat tmpLabels;


		for (int i = 0; i < dimsZ; i++) {
			ifs.read( (char*) &value, sizeof(value));
			tmpLabels.push_back(value);
		}
		transpose(tmpLabels, tmpLabels);
		labels.push_back(tmpLabels);

		Mat featureMatrix (dimsY,dimsX,CV_32F);
		for (int x = 0; x < dimsX; x++) {
			for (int y = 0; y < dimsY; y++) {
				ifs.read( (char*) &value, sizeof(value));
				featureMatrix.at<float>(y,x) = value;
			}
		}
		features.push_back(featureMatrix);
	}
}

//Atention, matrices must be in CV_32F format (single dimensional float matrix)
void MatrixTools::writeBinV2(string& filename, vector<cv::Mat>& features, cv::Mat& labels, bool append ){

    if (features.size() > 0){
	std::fstream binFile;
	binFile.open(filename.c_str(),std::ios::binary | std::ios::in | std::ios::out);

    bool isEmpty = binFile.peek() == std::ifstream::traits_type::eof();
    binFile.clear();
    binFile.seekg(0, std::ios::beg);
    binFile.seekp(0, std::ios::beg);

    if (isEmpty){
        binFile.open(filename.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
    }

    int signature,binV;

    int nSamples = (int)features.size();
    int dimsX = features.at(0).cols;
    int dimsY = features.at(0).rows;
    int dimsZ = labels.cols;

    if (!append || isEmpty){
        signature = BIN_SIGNATURE_INT;
        binV = BIN_VERSION;

        binFile.write((const char*) &signature, sizeof(int));
        binFile.write((const char*) &binV, sizeof(int));
        binFile.write((const char*) &nSamples, sizeof(int));

        binFile.write((const char*) &dimsX, sizeof(int));
        binFile.write((const char*) &dimsY, sizeof(int));
        binFile.write((const char*) &dimsZ, sizeof(int));

    } else {

        binFile.read( (char*) &signature, sizeof(signature));
        if (signature != BIN_SIGNATURE_INT){
            cerr << "Read error: Wrong signature" << endl;
            return;
        }

        binFile.read( (char*) &binV, sizeof(binV));
        if (binV != BIN_VERSION){
            cerr << "Read error: Wrong version" << endl;
            return;
        }
        int nSamplesExisting;
        binFile.read( (char*) &nSamplesExisting, sizeof(nSamplesExisting));
        int nSamplesExisting2 = nSamples + nSamplesExisting;
        binFile.read( (char*) &binV, sizeof(binV));

        binFile.clear();
        binFile.seekp(8);
        binFile.seekg(8);
        binFile.write((const char*) &nSamplesExisting2, sizeof(int));
        binFile.flush();
        binFile.seekp(0,std::ios::end);
        binFile.seekp(0,std::ios::end);
    }
	//std::cout << features.cols << " " << features.rows << endl;

	for (int s = 0; s < nSamples; s++) {

		float value;
		for (int i = 0; i < dimsZ; i++) {
			value = (float) labels.at<float>(s,i);
			binFile.write((const char*) &value, sizeof(float));
		}
		Mat featureSingle = features.at(s);
		for (int x = 0; x < dimsX; x++) {
			for (int y = 0; y < dimsY; y++) {
				value = featureSingle.at<float>(y,x);
				binFile.write((const char*) &value, sizeof(float));
			}
			binFile.flush();
		}
		binFile.flush();
	}

	binFile.close();
	}

}

//Atention, matrices must be in CV_32F format (single dimensional float matrix)
void MatrixTools::writeBinV3(string& filename, vector<cv::Mat>& features, cv::Mat& labels, bool append){
    int nSamples = (int)features.size();

    if (nSamples == 0)
        return;

	std::fstream binFile;
	binFile.open(filename.c_str(), std::ios::binary | std::ios::in | std::ios::out);

    bool isEmpty = binFile.peek() == std::ifstream::traits_type::eof();
    binFile.clear();
    binFile.seekg(0, std::ios::beg);
    binFile.seekp(0, std::ios::beg);

    if (isEmpty){
        binFile.open(filename.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
    }

    int signature,binV;

    if (!append || isEmpty){
        signature = BIN_SIGNATURE_INT;
        binV = NBIN_VERSION;

        binFile.write((const char*) &signature, sizeof(int));
        binFile.write((const char*) &binV, sizeof(int));
        binFile.write((const char*) &nSamples, sizeof(int));
    } else {
        binFile.read( (char*) &signature, sizeof(signature));
        if (signature != BIN_SIGNATURE_INT){
            cerr << "Read error: Wrong signature" << endl;
            return;
        }

        binFile.read( (char*) &binV, sizeof(binV));
        if (binV != NBIN_VERSION){
            cerr << "Read error: Wrong version" << endl;
            return;
        }
        int nSamplesExisting;
        binFile.read( (char*) &nSamplesExisting, sizeof(nSamplesExisting));
        int nSamplesExisting2 = nSamples + nSamplesExisting;
        binFile.read( (char*) &binV, sizeof(binV));

        binFile.clear();
        binFile.seekp(8);
        binFile.seekg(8);
        binFile.write((const char*) &nSamplesExisting2, sizeof(int));
        binFile.flush();
        binFile.seekp(0,std::ios::end);
        binFile.seekp(0,std::ios::end);
    }

	//std::cout << features.cols << " " << features.rows << endl;
	for (int s = 0; s < nSamples; s++) {

        int dimsX = features.at(s).cols;
        int dimsY = features.at(s).rows;
        int dimsZ = labels.cols;

        binFile.write((const char*) &dimsX, sizeof(int));
        binFile.write((const char*) &dimsY, sizeof(int));
        binFile.write((const char*) &dimsZ, sizeof(int));

		float value;
		for (int i = 0; i < dimsZ; i++) {
			value = (float) labels.at<float>(s,i);
			binFile.write((const char*) &value, sizeof(float));
		}
		Mat featureSingle = features.at(s);
		for (int x = 0; x < dimsX; x++) {
			for (int y = 0; y < dimsY; y++) {
				value = featureSingle.at<float>(y,x);
				binFile.write((const char*) &value, sizeof(float));
			}
			binFile.flush();
		}
		binFile.flush();
	}

	binFile.close();

}

void MatrixTools::readBinV3(string& file, vector<cv::Mat>& features, vector<vector<cv::KeyPoint> >& keypoints, cv::Mat& labels){

    std::vector<cv::Mat> descKeypoints;
    MatrixTools::readBinV3(file,descKeypoints,labels);

    for (uint i = 0; i < descKeypoints.size(); i++){
        Mat keypointsMat;
        Mat descriptorMat;
        vector<cv::KeyPoint> kp;

        Mat keypointsMatTmp = descKeypoints.at(i).colRange(0,7);
        Mat descriptorMatTmp = descKeypoints.at(i).colRange(7,descKeypoints.at(i).cols);

        keypointsMatTmp.copyTo(keypointsMat);
        descriptorMatTmp.copyTo(descriptorMat);

        MatrixTools::matToKeypoints(keypointsMat,kp);

        keypoints.push_back(kp);
        features.push_back(descriptorMat);
    }

}

void MatrixTools::writeBinV3(string& file, cv::Mat& features, vector<cv::KeyPoint>& keypoints, cv::Mat& labels, bool append){
    std::vector<cv::Mat> descKeypoints;

    Mat descKeypointsMat;
    Mat keypointsMat;

    MatrixTools::keypointsToMats(keypoints,keypointsMat);

    if (keypointsMat.empty() || features.empty())
    	return;

    cv::hconcat(keypointsMat,features,descKeypointsMat);
    descKeypoints.push_back(descKeypointsMat);
    MatrixTools::writeBinV3(file,descKeypoints,labels,append);
}

void MatrixTools::keypointsToMats(std::vector<cv::KeyPoint>& p, cv::Mat& m){
    for (uint i = 0; i < p.size(); i++){
        Mat m1;
        MatrixTools::keypointToMat(p.at(i), m1);
        m.push_back(m1);
    }
}

void MatrixTools::keypointToMat(cv::KeyPoint& p, cv::Mat& m){
    m = Mat(1,7,CV_32F);

    m.at<float>(0) = p.pt.x;
    m.at<float>(1) = p.pt.y;

    m.at<float>(2) = p.size;
    m.at<float>(3) = p.angle;
    m.at<float>(4) = p.response;
    m.at<float>(5) = p.octave;
    m.at<float>(6) = p.class_id;
}

void MatrixTools::matToKeypoint(cv::Mat& m, cv::KeyPoint& p){
    p = cv::KeyPoint(Point2f(m.at<float>(0),m.at<float>(1)), m.at<float>(2), m.at<float>(3), m.at<float>(4), m.at<float>(5), m.at<float>(6));
}

void MatrixTools::matToKeypoints(Mat& m, std::vector<cv::KeyPoint>& p){
    for (int i = 0; i < m.rows; i++){
        cv::KeyPoint p1;
        Mat m1 = m.row(i);
        MatrixTools::matToKeypoint(m1, p1);
        p.push_back(p1);
    }}
