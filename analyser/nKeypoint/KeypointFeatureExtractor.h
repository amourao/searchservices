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
#include "../../dataModel/NKeypoint.h"

#define KEYPOINT_FEATURES_NAME "KeypointFeatures"

using namespace cv;
using namespace std;

class KeypointFeatureExtractor: public IAnalyser
{
public:
	
	virtual ~KeypointFeatureExtractor()  {}

	IDataModel* getFeatures(string name){
		vector<pair<vector<float>,vector<float> > > features;
		extractFeatures(name, features);
		NKeypoint *key = new NKeypoint(name,getName(), features);
		return key;
	}

	virtual void extractFeatures(string filename, vector<pair<vector<float>,vector<float> > >& features){
		Mat source = imread(filename);
		vector< cv::KeyPoint> keypointsCv;
		Mat featuresMat;
		extractFeatures(source,keypointsCv, featuresMat);

		for(unsigned int i = 0; i < keypointsCv.size(); i++){
			cout << keypointsCv.at(i).pt.x << " " << keypointsCv.at(i).pt.y << " "<< keypointsCv.at(i).angle << " "<< keypointsCv.at(i).size << " "<< keypointsCv.at(i).response << " " << keypointsCv.at(i).octave << " " << keypointsCv.at(i).class_id  << endl;
			
			vector<float> point;
			point.push_back( keypointsCv.at(i).pt.x);
			point.push_back( keypointsCv.at(i).pt.y);
			point.push_back( keypointsCv.at(i).angle);
			point.push_back( keypointsCv.at(i).size);
			point.push_back( keypointsCv.at(i).response);
			point.push_back( keypointsCv.at(i).octave);
			point.push_back( keypointsCv.at(i).class_id);
			//keypoints.push_back(point);
			
			
			vector<float> featuresR;
			Mat m = featuresMat.row(i);
			MatrixTools::matToVector(m, featuresR);
			
			pair<vector<float>,vector<float> > pairP = make_pair(point,featuresR);
			
			features.push_back(pairP);
		}
	}
	
	virtual void extractFeatures(Mat& source,vector< cv::KeyPoint>& keypoints, Mat& features) = 0;
	
	virtual int getDescriptorSize() = 0;
	
	virtual string getName() = 0;
	
	virtual void* createType(string &typeId) = 0;
	
		IDataModel::type getType(){
		return IDataModel::NKEYPOINT;
	}
};

