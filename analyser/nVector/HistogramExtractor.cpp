#include "HistogramExtractor.h"

static HistogramExtractor histogramExtractorFactory;

HistogramExtractor::HistogramExtractor(){
	FactoryAnalyser::getInstance()->registerType("hsvHistogramExtractor",this);
}


HistogramExtractor::HistogramExtractor(int _binCount){
	binCount = _binCount;
	//featureChannel = _featureChannel;
}

HistogramExtractor::HistogramExtractor(string& _type){
    type = _type;
}

HistogramExtractor::HistogramExtractor(string& _type, map<string,string>& params){
    type = _type;
    if (params.size() == 0)
        return;

    binCount = atoi(params["binCount"].c_str());
}

HistogramExtractor::~HistogramExtractor(){

}

void* HistogramExtractor::createType(string& type){
	if (type == "hsvHistogramExtractor")
		return new HistogramExtractor(type);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;

}

void* HistogramExtractor::createType(string& type, map<string,string>& params){
    return new HistogramExtractor(type,params);
}


void HistogramExtractor::extractFeatures(Mat& src, Mat& dst){
	Mat result;
	Mat tmpResult;
	Mat hsv;
	cvtColor(src, hsv, CV_BGR2HSV);

	for (int i = 0; i < 3; i++){
		extractFeaturesSingle(hsv,tmpResult,i);
		transpose(tmpResult,tmpResult);
		if (result.empty())
			tmpResult.copyTo(result);
		else
			hconcat(result,tmpResult,result);

	}


	result.copyTo(dst);
	dst /= src.rows * src.cols;
}

void HistogramExtractor::extractFeaturesSingle(Mat& src, Mat& dst, int channel){
	int histSize[] = {binCount};

	float *range = (float*)malloc(sizeof(float)*2);

	range[0] = 0;

	if(channel == HUE_FEATURES){
		range[1] = 180;
	}else if(channel == SAT_FEATURES){
		range[1] = 256;
	}else if(channel == VAL_FEATURES){
		range[1] = 256;
	}


	const float* ranges[] = { range};
	int channels[] = {channel};

	calcHist( &src, 1, channels, Mat(), // do not use mask
		dst, 1, histSize, ranges,
		true, // the histogram is uniform
		false );
}

int HistogramExtractor::getFeatureVectorSize(){
	return binCount*3;
}

string HistogramExtractor::getName(){
	return type;
}

