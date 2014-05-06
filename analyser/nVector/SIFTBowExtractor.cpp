#include "SIFTBowExtractor.h"

static SIFTBowExtractor SIFTBowExtractorFactory;

SIFTBowExtractor::SIFTBowExtractor(){
	FactoryAnalyser::getInstance()->registerType("SIFTBowExtractor",this);
}

SIFTBowExtractor::SIFTBowExtractor(string& _type){
    type = _type;
}

SIFTBowExtractor::SIFTBowExtractor(string& _type, map<string,string>& params){
    type = _type;

    if (params.size() > 0){
        loadVocabulary(params["vocabulary"]);
		detector = new SiftFeatureDetector(0, 4, 0.04, 10, 1.6);
		bowMatcher = new BOWImgDescriptorExtractor(DescriptorExtractor::create("SIFT"),DescriptorMatcher::create("BruteForce"));
		bowMatcher->setVocabulary(vocabulary);
	}
}

SIFTBowExtractor::~SIFTBowExtractor(){
}

void* SIFTBowExtractor::createType(string& type){
	if (type == "SIFTBowExtractor")
		return new SIFTBowExtractor(type);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* SIFTBowExtractor::createType(string& type, map<string,string>& params){
    return new SIFTBowExtractor(type,params);
}

void SIFTBowExtractor::loadVocabulary(string trainDataFile){
    Mat l;
	MatrixTools::readBinV2(trainDataFile,vocabulary,l);
}

void SIFTBowExtractor::load(string xmlDataFile){

}
void SIFTBowExtractor::save(string xmlDataFile){
}

void SIFTBowExtractor::extractFeatures(Mat& src, Mat& dst){
	vector<KeyPoint> keypoints;
	detector->detect(src, keypoints);
	bowMatcher->compute(src, keypoints, dst);
}

int SIFTBowExtractor::getFeatureVectorSize(){
	return bowMatcher->descriptorSize();
}

string SIFTBowExtractor::getName(){
	return "SIFTBowExtractor";
}
