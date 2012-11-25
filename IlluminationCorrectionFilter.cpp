#include "IlluminationCorrectionFilter.h"


IlluminationCorrectionFilter::IlluminationCorrectionFilter(int width_, int height_, cv::Rect right, cv::Rect left){
	illumBaseLeft = left;
	illumBaseRight = right;
	height = height_;
	width = width_;

	createIlluminationFilters(width, height);
}

IlluminationCorrectionFilter::IlluminationCorrectionFilter(cv::Rect right, cv::Rect left){
	illumBaseLeft = left;
	illumBaseRight = right;
}

IlluminationCorrectionFilter::~IlluminationCorrectionFilter(){
}


void IlluminationCorrectionFilter::applyFilter(Mat& src, Mat& dst){
	Mat singleDim;
	if (src.channels() > 1){
		cvtColor(src, dst, COLOR_RGB2GRAY);
	} else if (src.type() != CV_8U){
		src.convertTo(dst,CV_8U);
	} else {
		src.copyTo(dst);
	}
	
	Mat rightROIMat(src,illumBaseRight);
	Mat leftROIMat(src,illumBaseLeft);

	 Scalar rightMean = mean(rightROIMat);
	 Scalar leftMean = mean(leftROIMat);

	 if (abs(leftMean(0)-rightMean(0)) > ILLUMINATION_THRESHOLD){

	 float mean = abs(leftMean(0)+rightMean(0))/2;

	 //cout << mean << " " << rightMean(0) << " " << leftMean(0) << endl;


	 Mat right;
	 Mat left;
	 
	 src.copyTo(right);
	 src.copyTo(left);

	 right.convertTo(right,CV_32F);
	 left.convertTo(left,CV_32F);


	 if (rightMean(0) > leftMean(0)){
		 right -= mean*illuminationFilterDarkFirst;
		 left += mean*illuminationFilterBrightFirst;
	 } else {
		 right += mean*illuminationFilterDarkFirst;
		 left -= mean*illuminationFilterBrightFirst;
	 }

	 	
	 right.convertTo(right,CV_8U);
	 left.convertTo(left,CV_8U);

	 Mat result = (right+left)/2;
	 
	 Mat hist;

	 equalizeHist(result,dst);
	  dst.convertTo(dst,CV_8U);
	 //imwrite("_left.bmp",left);
	 //imwrite("_right.bmp",right);
	 //imwrite("_result.bmp",result);
	 //imwrite("_hist.bmp",hist);
	 //imwrite("_original.bmp",image);

	 }else {
		 equalizeHist(dst,dst);
	 }

}	 

void IlluminationCorrectionFilter::createIlluminationFilters(int width, int height){
	 illuminationFilterBrightFirst = Mat(height,width,CV_32F);
	 illuminationFilterDarkFirst = Mat(height,width,CV_32F);

	 
	 for (int i = 0; i < width ; i++){
		 float value = 1/(exp((width/2.0-i)/10)+1);
		 for (int j = 0; j < height; j++){
			 illuminationFilterBrightFirst.at<float>(j,i) = value;
			 illuminationFilterDarkFirst.at<float>(j,i) = 1-value;
		 }
	 }

	 //imwrite("demo.bmp",illuminationFilterBrightFirst*256);
	 //imwrite("demo2.bmp",illuminationFilterDarkFirst*256);
 }