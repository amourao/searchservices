#include "SIFTExtractor.h"

static SIFTExtractor surfExtractorFactory;

SIFTExtractor::SIFTExtractor(){
	FactoryAnalyser::getInstance()->registerType("sift",this);
}

SIFTExtractor::SIFTExtractor(string& typeId){
    type = typeId;
}

SIFTExtractor::SIFTExtractor(string& typeId, map<string,string>& params){
    type = typeId;

    if(params.size() == 0)
        return;

    detector = SiftFeatureDetector::create(atoi(params["nFeatures"].c_str()),atoi(params["nOctaveLayers"].c_str()), atof(params["contrastThreshold"].c_str()), atof(params["edgeThreshold"].c_str()), atof(params["sigma"].c_str()));
    extractor = SiftDescriptorExtractor::create();
}

SIFTExtractor::SIFTExtractor(int nFeatures, int nOctaveLayers, double contrastThreshold, double edgeThreshold, double sigma){
	detector = SiftFeatureDetector::create(nFeatures , nOctaveLayers, contrastThreshold, edgeThreshold, sigma);
	extractor = SiftDescriptorExtractor::create();
}

SIFTExtractor::~SIFTExtractor(){

}


void* SIFTExtractor::createType(string& type){
	if (type == "sift")
		return new SIFTExtractor(type);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* SIFTExtractor::createType(string& type, map<string,string>& params){
	return new SIFTExtractor(type,params);
}

void SIFTExtractor::extractFeatures(Mat& src,vector< cv::KeyPoint>& keypoints, Mat& features){
	detector->detect( src, keypoints );
	extractor->compute( src, keypoints, features);
}


int SIFTExtractor::getDescriptorSize(){
	return detector->descriptorSize();
}


string SIFTExtractor::getName(){
	return "SIFTExtractor";
}
