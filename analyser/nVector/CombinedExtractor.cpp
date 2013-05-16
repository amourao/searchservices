#include "CombinedExtractor.h"

static CombinedExtractor combinedExtratorExtractorFactory;

CombinedExtractor::CombinedExtractor(){
	FactoryAnalyser::getInstance()->registerType("SEG_LBP_HIST",this);
}

void* CombinedExtractor::createType(string& type){
	//TODO
	if (type == "SEG_LBP_HIST")
		return new CombinedExtractor(8,5,5,true);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

CombinedExtractor::CombinedExtractor(int _binCount, int _horizontalDivisions, int _verticalDivisions, bool _useCenterRegion){
	she = new SegmentedHistogramExtractor(_binCount,_horizontalDivisions, _verticalDivisions,_useCenterRegion);
	lbpe = new LBPExtractor(_binCount,_horizontalDivisions, _verticalDivisions,_useCenterRegion);
}


CombinedExtractor::~CombinedExtractor(){

}

void CombinedExtractor::extractFeatures(Mat& src, Mat& dst){
	Mat featuresRow1;
	Mat featuresRow2;
	she->extractFeatures(src, featuresRow1);
	lbpe->extractFeatures(src, featuresRow2);
	hconcat(featuresRow1,featuresRow2,dst);
}

int CombinedExtractor::getFeatureVectorSize(){
	return she->getFeatureVectorSize() + lbpe->getFeatureVectorSize();
}

string CombinedExtractor::getName(){
	return "CombinedExtractor";
}

