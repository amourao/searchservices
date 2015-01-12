#include "LBPExtractor.h"

static LBPExtractor lbpExtractorFactory;

LBPExtractor::LBPExtractor(){
	FactoryAnalyser::getInstance()->registerType("lbpExtractor",this);

}

LBPExtractor::LBPExtractor(string& _type){
	type = _type;
}

LBPExtractor::LBPExtractor(string& _type, map<string, string>& params){
	type = _type;

	if (params.size() == 0)
        return;

    binCount = atoi(params["binCount"].c_str());
    horizontalDivisions = atoi(params["horizontalDivisions"].c_str());
    verticalDivisions = atoi(params["verticalDivisions"].c_str());
    useCenterRegion = params["useCenterRegion"] == "true";

}

LBPExtractor::LBPExtractor(int _binCount, int _horizontalDivisions, int _verticalDivisions, bool _useCenterRegion){
	binCount = _binCount;
	horizontalDivisions=_horizontalDivisions;
	verticalDivisions =_verticalDivisions;
	useCenterRegion = _useCenterRegion;
}


LBPExtractor::~LBPExtractor(){

}

void* LBPExtractor::createType(string& type){
	//TODO
	if (type == "lbpExtractor")
		return new LBPExtractor(type);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* LBPExtractor::createType(string& type, map<string, string>& params){
    return new LBPExtractor(type,params);
}

void LBPExtractor::extractFeatures(Mat& src, Mat& dst){
	Mat nSrc;
	if (src.channels() > 1){
		cv::cvtColor(src, nSrc, COLOR_RGB2GRAY);
	} else if (src.type() != CV_8U){
		src.convertTo(nSrc,CV_8U);
	}

	Mat results,result;

	lbp::OLBP(nSrc,results);

	normalize(results, results, 0, 255, NORM_MINMAX, CV_8UC1);

	int histSize = binCount; //from 0 to 255
	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 } ; //the upper boundary is exclusive
	const float* histRange = { range };
	bool uniform = true;
	bool accumulate = false;

	int width = results.cols;
	int height = results.rows;
	double divisionSizeW = width/(double)(horizontalDivisions+1);
	double divisionSizeH = height/(double)(verticalDivisions+1);
	for (int i = 0; i <= horizontalDivisions; i++){
		for (int j = 0; j <= verticalDivisions; j++){
			Mat tmpResult;

			cv::Rect roi(j*divisionSizeW,i*divisionSizeH,min(width-j*divisionSizeW,divisionSizeW),min(height-i*divisionSizeH,divisionSizeH));
			Mat roiImage(results,roi);

			calcHist(&roiImage, 1, 0, Mat(), tmpResult, 1, &histSize, &histRange, uniform, accumulate );

			transpose(tmpResult,tmpResult);

			tmpResult /= width * height;
			if (result.empty())
				tmpResult.copyTo(result);
			else
				hconcat(result,tmpResult,result);
		}
	}
	if (useCenterRegion){

		Mat tmpResult;
		cv::Rect roi(width*0.3,height*0.3,width*0.4,height*0.4);
		Mat roiImage(results,roi);

		calcHist(&roiImage, 1, 0, Mat(), tmpResult, 1, &histSize, &histRange, uniform, accumulate );
		transpose(tmpResult,tmpResult);

		tmpResult /= width * height;
		hconcat(result,tmpResult,result);
	}



	result.copyTo(dst);

	//std::cout << result << std::endl;
	//imshow("a",results);
	//waitKey();
}

void LBPExtractor::applyFilter(Mat& src, Mat& dst){
	Mat nSrc;
	if (src.channels() > 1){
		cv::cvtColor(src, nSrc, COLOR_RGB2GRAY);
	} else if (src.type() != CV_8U){
		src.convertTo(nSrc,CV_8U);
	}

	Mat results;

	lbp::OLBP(nSrc,results);

	normalize(results, results, 0, 255, NORM_MINMAX, CV_8UC1);

	results.copyTo(dst);
}

int LBPExtractor::getFeatureVectorSize(){
	if (useCenterRegion)
		return binCount*((horizontalDivisions+1)*(verticalDivisions+1)+1);
	else
		return binCount*(horizontalDivisions+1)*(verticalDivisions+1);
}

string LBPExtractor::getName(){
	return "LBPExtractor";
}

