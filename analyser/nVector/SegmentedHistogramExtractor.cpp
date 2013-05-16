#include "SegmentedHistogramExtractor.h"

static SegmentedHistogramExtractor segHistogramExtractorFactory;

SegmentedHistogramExtractor::SegmentedHistogramExtractor(){
	FactoryAnalyser::getInstance()->registerType("SegHistogram16",this);
	FactoryAnalyser::getInstance()->registerType("SegHistogram8",this);
}

void* SegmentedHistogramExtractor::createType(string& type){
	//TODO
	if (type == "SegHistogram8"){
		return new SegmentedHistogramExtractor(8,2,2);
	}else if (type == "SegHistogram16"){
		return new SegmentedHistogramExtractor(16,2,2);
	}cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

SegmentedHistogramExtractor::SegmentedHistogramExtractor(int _binCount, int _horizontalDivisions, int _verticalDivisions){
	binCount = _binCount;
	horizontalDivisions=_horizontalDivisions;
	verticalDivisions =_verticalDivisions;
	useCenterRegion = false;
	singleHistogramExtractor = new HistogramExtractor(binCount);
}

SegmentedHistogramExtractor::SegmentedHistogramExtractor(int _binCount, int _horizontalDivisions, int _verticalDivisions, bool _useCenterRegion){
	binCount = _binCount;
	horizontalDivisions=_horizontalDivisions;
	verticalDivisions =_verticalDivisions;
	useCenterRegion = _useCenterRegion;
	singleHistogramExtractor = new HistogramExtractor(binCount);
}


SegmentedHistogramExtractor::~SegmentedHistogramExtractor(){

}

void SegmentedHistogramExtractor::extractFeatures(Mat& src, Mat& dst){
	Mat result;
	Mat tmpResult;
	int width = src.cols;
	int height = src.rows;
	double divisionSizeW = width/(double)(horizontalDivisions+1);
	double divisionSizeH = height/(double)(verticalDivisions+1); 
	for (int i = 0; i <= horizontalDivisions; i++){
		for (int j = 0; j <= verticalDivisions; j++){
			cv::Rect roi(j*divisionSizeW,i*divisionSizeH,min(width-j*divisionSizeW,divisionSizeW),min(height-i*divisionSizeH,divisionSizeH));
			//cout << i << " " << j << " " << endl;
			//cout << height << " " << width<< " " <<divisionSizeH << " " <<divisionSizeW<< endl;
			//cout << roi << endl;
			Mat roiImage(src,roi);
			singleHistogramExtractor->extractFeatures(roiImage,tmpResult);
			if (result.empty())
				tmpResult.copyTo(result);
			else
				hconcat(result,tmpResult,result);
		}		
	}
	if (useCenterRegion){
		
		Mat tmpResult;
		cv::Rect roi(width*0.3,height*0.3,width*0.4,height*0.4);
		Mat roiImage(src,roi);
		
		singleHistogramExtractor->extractFeatures(roiImage,tmpResult);
		
		hconcat(result,tmpResult,result);
	}
	result.copyTo(dst);
}


int SegmentedHistogramExtractor::getFeatureVectorSize(){
	return singleHistogramExtractor->getFeatureVectorSize()*(horizontalDivisions+1)*(verticalDivisions+1);
}

string SegmentedHistogramExtractor::getName(){
	return "SegmentedHistogramExtractor";
}


