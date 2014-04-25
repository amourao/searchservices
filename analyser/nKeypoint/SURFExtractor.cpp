#include "SURFExtractor.h"

static SURFExtractor surfExtractorFactory;

SURFExtractor::SURFExtractor(){
	FactoryAnalyser::getInstance()->registerType("surf",this);
}

SURFExtractor::SURFExtractor(string& typeId){
    type = typeId;
}

SURFExtractor::SURFExtractor(string& typeId, map<string,string>& params){
    type = typeId;

    if(params.size() == 0)
        return;

    detector = SurfFeatureDetector(atof(params["hessianThreshold"].c_str()) , atof(params["nOctaves"].c_str()), atof(params["nOctaveLayers"].c_str()), params["extended"] == "true", params["upright"] == "true");
}

SURFExtractor::SURFExtractor(double hessianThreshold, int nOctaves, int nOctaveLayers, bool extended, bool upright){
	detector = SurfFeatureDetector( hessianThreshold , nOctaves, nOctaveLayers, extended, upright);
}

SURFExtractor::~SURFExtractor(){
}


void* SURFExtractor::createType(string& type){
	if (type == "surf")
		return new SURFExtractor(type);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* SURFExtractor::createType(string& type, map<string,string>& params){
	return new SURFExtractor(type,params);
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
	features.convertTo(features,CV_32F);
}


int SURFExtractor::getDescriptorSize(){
	return detector.descriptorSize();
}


string SURFExtractor::getName(){
	return "SURFExtractor";
}
