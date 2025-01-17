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
#include "../../dataModel/NTag.h"

#define TAG_FEATURES_NAME "TagFeatures"

using namespace cv;
using namespace std;

class TagFeatureExtractor: public IAnalyser
{
public:
	
	virtual ~TagFeatureExtractor()  {}

	IDataModel* getFeatures(string name){
		map<string,float> features;
		extractFeatures(name, features);
		NTag *tag = new NTag(name,features);
		return tag;
	}

	virtual void extractFeatures(string filename, map<string,float>& features){
		Mat source = imread(filename);
		extractFeatures(source,features);
	}
	
	virtual void extractFeatures(Mat& source, map<string,float>& features) = 0;

	virtual void train(string trainFile) = 0;
	
	virtual string test(string testFile) = 0;

	virtual string crossValidate(string testFile) = 0;

	virtual string getName() = 0;
	
	virtual void* createType(string &typeId) = 0;
	
	IDataModel::type getType(){
		return IDataModel::NTAG;
	}
};

