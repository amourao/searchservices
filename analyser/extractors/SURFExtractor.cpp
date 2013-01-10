#include "SURFExtractor.h"


SURFExtractor::SURFExtractor(double hessianThreshold, int nOctaves, int nOctaveLayers, bool extended, bool upright){
	//detector = FeatureDetector::create(
	detector = SurfFeatureDetector( hessianThreshold , nOctaves, nOctaveLayers, extended, upright);
}

SURFExtractor::~SURFExtractor(){ 
}

void SURFExtractor::extractFeatures(Mat& src,vector< cv::KeyPoint>& keypoints, Mat& features){
	Mat srcGray;
	if (src.channels() > 1){
		cv::cvtColor(src, srcGray, COLOR_RGB2GRAY);
	} else if (src.type() != CV_8U){
		src.convertTo(srcGray,CV_8U);
	} 
	detector.detect( srcGray, keypoints );
	extractor.compute( srcGray, keypoints, features);
}


int SURFExtractor::getDescriptorSize(){
	return detector.descriptorSize();
}


string SURFExtractor::getName(){
	return "SURFExtractor";
}

void* SURFExtractor::createType(string &typeId){
	return new SURFExtractor(400);
}
