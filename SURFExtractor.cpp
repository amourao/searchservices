#include "SURFExtractor.h"


SURFExtractor::SURFExtractor(double hessianThreshold, int nOctaves, int nOctaveLayers, bool extended, bool upright){
	detector = SurfFeatureDetector( hessianThreshold , nOctaves, nOctaveLayers, extended, upright);
}

SURFExtractor::~SURFExtractor(){ 
}

void SURFExtractor::extractFeatures(Mat& src, Mat& dst){
	Mat srcGray;
	if (src.channels() > 1){
		cv::cvtColor(src, srcGray, COLOR_RGB2GRAY);
	} else if (src.type() != CV_8U){
		src.convertTo(srcGray,CV_8U);
	} 
	std::vector<KeyPoint> keypoints;
	detector.detect( srcGray, keypoints );
	extractor.compute( srcGray, keypoints, dst);
}


int SURFExtractor::getFeatureVectorSize(){
	return detector.descriptorSize();
}
