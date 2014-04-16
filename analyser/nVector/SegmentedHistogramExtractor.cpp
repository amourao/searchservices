#include "SegmentedHistogramExtractor.h"

static SegmentedHistogramExtractor segHistogramExtractorFactory;

SegmentedHistogramExtractor::SegmentedHistogramExtractor(){
	FactoryAnalyser::getInstance()->registerType("hsvSegHistogramExtractor",this);
}

SegmentedHistogramExtractor::SegmentedHistogramExtractor(string& _type){
    type = _type;
}

SegmentedHistogramExtractor::SegmentedHistogramExtractor(string& _type, map<string, string>& params){
    type = _type;

    if (params.size() == 0)
        return;

    int _binCount = atoi(params["binCount"].c_str());
    int _horizontalDivisions = atoi(params["horizontalDivisions"].c_str());
    int _verticalDivisions = atoi(params["verticalDivisions"].c_str());
    bool _useCenterRegion = params["useCenterRegion"] == "true";

    init(_binCount, _horizontalDivisions, _verticalDivisions, _useCenterRegion);
}

SegmentedHistogramExtractor::SegmentedHistogramExtractor(int _binCount, int _horizontalDivisions, int _verticalDivisions, bool _useCenterRegion){
	init(_binCount, _horizontalDivisions, _verticalDivisions, _useCenterRegion);
}


void* SegmentedHistogramExtractor::createType(string& type){
	if (type == "hsvSegHistogramExtractor"){
		return new SegmentedHistogramExtractor(type);
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* SegmentedHistogramExtractor::createType(string& type, map<string, string>& params){
    return new SegmentedHistogramExtractor(type,params);
}

void SegmentedHistogramExtractor::init(int _binCount, int _horizontalDivisions, int _verticalDivisions, bool _useCenterRegion){
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


