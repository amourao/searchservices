#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <string>
#include <vector>
#include <utility>

#include "../tools/MatrixTools.h"
#include "../IAnalyser.h"
#include "../../dataModel/IDataModel.h"
#include "../../dataModel/NRoi.h"

#define ROI_FEATURES_NAME "RoiFeatures"

using namespace cv;
using namespace std;

class RoiFeatureExtractor: public IAnalyser
{
public:

	virtual ~RoiFeatureExtractor()  {}

	IDataModel* getFeatures(string name){
		map<string,region> features;
		extractFeatures(name, features);
		NRoi* roi = new NRoi(name, features);
		return roi;
	}

	virtual void extractFeatures(string filename, map<string,region>& features){
		cv::Mat source = imread(filename);
		extractFeatures(source,features);
	}

	virtual void extractFeatures(cv::Mat& source, map<string,region>& features) = 0;

	virtual void train(string trainFile) = 0;

	virtual string test(string testFile) = 0;

	virtual string crossValidate(string testFile) = 0;

	virtual string getName() = 0;

	virtual void* createType(string &typeId) = 0;

	IDataModel::type getType(){
		return IDataModel::NROI;
	}
};

