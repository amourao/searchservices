#include "LBPExtractor.h"

static LBPExtractor lbpExtractorFactory;

LBPExtractor::LBPExtractor(){
	FactoryAnalyser::getInstance()->registerType("LBP",this);
	
}

void* LBPExtractor::createType(string& type){
	//TODO
	if (type == "LBP")
		return new LBPExtractor(8,5,5,true);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

LBPExtractor::LBPExtractor(int _binCount, int _horizontalDivisions, int _verticalDivisions, bool _useCenterRegion){
	binCount = _binCount;
	horizontalDivisions=_horizontalDivisions;
	verticalDivisions =_verticalDivisions;
	useCenterRegion = _useCenterRegion;
}


LBPExtractor::~LBPExtractor(){

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

int LBPExtractor::getFeatureVectorSize(){
	if (useCenterRegion)
		return binCount*((horizontalDivisions+1)*(verticalDivisions+1)+1);
	else
		return binCount*(horizontalDivisions+1)*(verticalDivisions+1);
}

string LBPExtractor::getName(){
	return "LBPExtractor";
}

