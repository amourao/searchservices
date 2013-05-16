#include "GaborExtractor.h"

static GaborExtractor gaborExtractorFactory;

GaborExtractor::GaborExtractor(){
	FactoryAnalyser::getInstance()->registerType("GaborFace",this);
	FactoryAnalyser::getInstance()->registerType("GaborGlobal",this);
}

void* GaborExtractor::createType(string& type){
	//TODO
	if (type == "GaborFace"){
		vector<cv::Rect> rectangleRois1 = vector<cv::Rect>();
		rectangleRois1.push_back(cv::Rect(0,0,46,64));
		rectangleRois1.push_back(cv::Rect(46,64,46,112-64));
		rectangleRois1.push_back(cv::Rect(46,0,46,64));
		rectangleRois1.push_back(cv::Rect(0,64,46,112-64));
		rectangleRois1.push_back(cv::Rect(0,10,92,30));
		rectangleRois1.push_back(cv::Rect(20,65,52,30));
		return new GaborExtractor(112,92,4,8,rectangleRois1);
		
	}else if (type == "GaborGlobal"){
		return new GaborExtractor(640,480,4,8);
		
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;

	return NULL;
}

GaborExtractor::GaborExtractor(int _imageW, int _imageH, int _nScales, int _nOrientations, vector<cv::Rect> rectangularAreas, int _minWaveLength,int _mult, double _sigmaOnf, double _dThetaOnSigma){

	// If no rectangles are given, assume the full image must be used
	if (rectangularAreas.empty()){
		rectangularAreas.push_back(cv::Rect(0,0,_imageW,_imageH));
	}

	rectangleRois = rectangularAreas;

	imageW = _imageW;
	imageH = _imageH;
	nScales = _nScales;
	nOrientations = _nOrientations;
	minWaveLength = _minWaveLength;
	mult = _mult;
	sigmaOnf = _sigmaOnf;
	dThetaOnSigma = _dThetaOnSigma;

	illumFilter = IlluminationCorrectionFilter(imageW,imageH);

	buildFilters();
	
}

GaborExtractor::~GaborExtractor(){}

int GaborExtractor::getFeatureVectorSize(){
	return filters.size()*rectangleRois.size()*2;
}

void GaborExtractor::preProcess(Mat& src, Mat& dst){
	Mat result;
	Mat singleDim;
	if (src.channels() > 1){
		cv::cvtColor(src, src, COLOR_RGB2GRAY);
	} else if (src.type() != CV_8U){
		src.convertTo(src,CV_8U);
	} 
	dst = src;
	
	//illumFilter.applyFilter(src,dst);
}

void GaborExtractor::applyFilter(Mat& image, Mat& dst){
cout << "tmp5" << endl;
getchar();
	preProcess(image,image);
	
	//int i = 0;
	//double rows = image.rows;
	//double cols = image.cols;
	dst = Mat::zeros(image.rows,image.cols,CV_32F);
	Mat imagefft;

	Mat ifft;

	fftw_complex* fft = newFFTW(image);

	for (unsigned int k = 0; k < filters.size(); k++){

			Mat filter = filters.at(k);

			fftw_complex* tmpResultC = (fftw_complex* )fftw_malloc(sizeof(fftw_complex)*filter.rows*filter.cols);

			int l = 0;
			for (int i = 0; i < filter.rows; i++){
				for (int j = 0; j < filter.cols; j++){
					tmpResultC[l][0] = fft[l][0] * filter.at<float>(i,j);
					tmpResultC[l][1] = fft[l][1] * filter.at<float>(i,j);
					l++;
				}
			}
			
			Mat tmpResult = newIFFTW(tmpResultC,filter.cols,filter.rows);

			fftw_free( tmpResultC );
			dst += tmpResult;
	}
	fftw_free( fft );
	normalize(dst, dst, 0,255, CV_MINMAX);

	//stringstream sst;
	//sst << "D:\\Datasets\\tmpEmotion\\" << tmp2++  <<".bmp";
	//imwrite(sst.str(),result);

	//dst = dst.reshape(1,1);
	dst.convertTo(dst,CV_8U);


}

void GaborExtractor::extractFeatures(Mat& image, Mat& result){
	cout << "tmp6" << endl;
	getchar();
	//int rectanglesLength=rectangleRois.size();

	// 2 quer dizer que por cada rectângulo é calculada a média e desvio padrão
	//int i = 0;
	//double rows = image.rows;
	//double cols = image.cols;

	result = Mat::zeros(1,getFeatureVectorSize(),CV_32F);
	Mat imagefft;

	Mat ifft;

	fftw_complex* fft = newFFTW(image);


	int pos = 0;
	for (unsigned int k = 0; k < filters.size(); k++){


			fftw_complex* tmpResultC = (fftw_complex* )fftw_malloc(sizeof(fftw_complex)*filters.at(k).rows*filters.at(k).cols);

			int l = 0;
			for (int i = 0; i < filters.at(k).rows; i++){
				for (int j = 0; j < filters.at(k).cols; j++){
					tmpResultC[l][0] = fft[l][0] * filters.at(k).at<float>(i,j);
					tmpResultC[l][1] = fft[l][1] * filters.at(k).at<float>(i,j);
					l++;
				}
			}
			
			Mat tmpResult = newIFFTW(tmpResultC,filters.at(k).cols,filters.at(k).rows);

			fftw_free( tmpResultC );

			for (unsigned int i = 0; i < rectangleRois.size(); i++){
				Scalar_<double> mean, stdDev;
				Mat tmpResultROI = tmpResult(rectangleRois[i]);
				meanStdDev(tmpResultROI,mean,stdDev);
				float m = mean(0);
				float s =  stdDev(0);
				result.at<float>(0,pos++) = m;
				result.at<float>(0,pos++) = s;
			}
	}
	fftw_free( fft );
	//normalize(result, result, 0,1, CV_MINMAX);
}

void GaborExtractor::meshGrid(double minX, double maxX, double minY, double maxY, int rows, int cols, Mat& x, Mat& y){
	double stepX = (maxX-minX)/(cols-1);
	double stepY = (maxY-minY)/(rows-1);

	for (int i = 0; i < rows; i++){
		for (int j = 0; j < cols; j++){
			x.at<float>(i,j) = minX+j*stepX;
			y.at<float>(i,j) = minY+i*stepY;;
		}
	}

}

// Rearrange the quadrants of Fourier image so that the origin is at
// the image center
// src & dst arrays of equal size & type
void GaborExtractor::cvShiftDFT(CvArr * src_arr, CvArr * dst_arr )
{
	CvMat * tmp;
	CvMat q1stub, q2stub;
	CvMat q3stub, q4stub;
	CvMat d1stub, d2stub;
	CvMat d3stub, d4stub;
	CvMat * q1, * q2, * q3, * q4;
	CvMat * d1, * d2, * d3, * d4;

	CvSize size = cvGetSize(src_arr);
	CvSize dst_size = cvGetSize(dst_arr);
	int cx, cy;

	if(dst_size.width != size.width ||
		dst_size.height != size.height){
			cvError( CV_StsUnmatchedSizes, "cvShiftDFT", "Source and Destination arrays must have equal sizes", __FILE__, __LINE__ );
	}

	if(src_arr==dst_arr){
		tmp = cvCreateMat(size.height/2, size.width/2, cvGetElemType
			(src_arr));
	}

	cx = size.width/2;
	cy = size.height/2; // image center

	q1 = cvGetSubRect( src_arr, &q1stub, cvRect(0,0,cx, cy) );
	q2 = cvGetSubRect( src_arr, &q2stub, cvRect(cx,0,cx,cy) );
	q3 = cvGetSubRect( src_arr, &q3stub, cvRect(cx,cy,cx,cy) );
	q4 = cvGetSubRect( src_arr, &q4stub, cvRect(0,cy,cx,cy) );
	d1 = cvGetSubRect( src_arr, &d1stub, cvRect(0,0,cx,cy) );
	d2 = cvGetSubRect( src_arr, &d2stub, cvRect(cx,0,cx,cy) );
	d3 = cvGetSubRect( src_arr, &d3stub, cvRect(cx,cy,cx,cy) );
	d4 = cvGetSubRect( src_arr, &d4stub, cvRect(0,cy,cx,cy) );

	if(src_arr!=dst_arr){
		if( !CV_ARE_TYPES_EQ( q1, d1 )){
			cvError( CV_StsUnmatchedFormats, "cvShiftDFT", "Source and Destination arrays must have the same format", __FILE__,
				__LINE__ );
		}
		cvCopy(q3, d1, 0);
		cvCopy(q4, d2, 0);
		cvCopy(q1, d3, 0);
		cvCopy(q2, d4, 0);
	}
	else{
		cvCopy(q3, tmp, 0);
		cvCopy(q1, q3, 0);
		cvCopy(tmp, q1, 0);
		cvCopy(q4, tmp, 0);
		cvCopy(q2, q4, 0);
		cvCopy(tmp, q2, 0);
	}
}

fftw_complex* GaborExtractor::newFFTW(Mat image){
	fftw_complex    *data_in;    
	fftw_complex    *fft;  
	fftw_plan       plan_f;

	int             width, height;
	int             i, j, k;
	Mat result (image.rows,image.cols,image.type());

	width  	  = image.cols;
	height 	  = image.rows;
	//step = 0;

	data_in = ( fftw_complex* )fftw_malloc( sizeof( fftw_complex ) * width * height );
	fft     = ( fftw_complex* )fftw_malloc( sizeof( fftw_complex ) * width * height );

	plan_f = fftw_plan_dft_2d( height , width, data_in, fft,  FFTW_FORWARD,  FFTW_ESTIMATE );

	k = 0;
	for( i = 0; i < height  ; i++ ) {
		for( j = 0 ; j < width ; j++ ) {
			data_in[k][0] = ( double )image.at<uchar>(i,j);
			data_in[k][1] = 0.0;
			k++;
		}
	}

	fftw_execute( plan_f );

	fftw_destroy_plan( plan_f );
	fftw_free( data_in );


	return fft;
}

Mat GaborExtractor::newIFFTW(fftw_complex* image, int width, int height){

	fftw_complex    *ifft;    
	fftw_plan       plan_b;

	Mat result (height,width,CV_32F);

	ifft    = ( fftw_complex* )fftw_malloc( sizeof( fftw_complex ) * width * height );

	plan_b = fftw_plan_dft_2d(  height , width, image,ifft, FFTW_BACKWARD, FFTW_ESTIMATE);

	fftw_execute( plan_b );

	/* normalize IFFT result */
	for(int i = 0 ; i < ( width * height ) ; i++ ) {
		ifft[i][0] /= ( double )( width * height );
		ifft[i][1] /= ( double )( width * height );
	}


	int k = 0;
	/* copy IFFT result to img2's data */
	for(int i = 0; i <  height ; i++ ) {
		for(int j = 0 ; j < width ; j++ ) {
			result.at<float>(i,j) = ( float )sqrt(pow(ifft[k][0],2) + pow(ifft[k][1],2));
			k++;
		}
	}

	fftw_destroy_plan( plan_b );
	fftw_free( ifft );

	return result;
}




void GaborExtractor::buildFilters(){
	filters.clear();
	//int i = 0;
	double rows = imageH;
	double cols = imageW;

	Mat x(rows,cols,CV_32F);
	Mat y(rows,cols,CV_32F);

	meshGrid((-cols/2)/cols,(cols/2-1)/cols,(-rows/2)/rows,(rows/2-1)/rows,rows,cols,x,y);

	Mat tmpX;
	Mat tmpY;

	pow(x, 2, tmpX);
	pow(y, 2, tmpY);

	Mat radius = tmpX + tmpY;
	pow(radius, 0.5, radius);

	radius.at<float>(floor(rows/2+1),floor(cols/2+1)) = 1;

	Mat sintheta = Mat::zeros(x.rows,x.cols,x.type());
	Mat costheta = Mat::zeros(x.rows,x.cols,x.type());

	for (int i = 0; i < x.rows; i++){
		for (int j = 0; j < x.cols; j++){
			float xValue = x.at<float>(i,j);
			float yValue = y.at<float>(i,j);
			float thetaValue = atan2(-yValue,xValue);
			sintheta.at<float>(i,j) = sin(thetaValue);
			costheta.at<float>(i,j) = cos(thetaValue);
		}
	}

	double thetaSigma = PI/nOrientations/dThetaOnSigma;

	for (int o = 1; o<=nOrientations; o++){

		double angl = (o-1)*PI/nOrientations;     
		double wavelength = minWaveLength;  

		Mat ds = sintheta * cos(angl) - costheta * sin(angl);
		Mat dc = costheta * cos(angl) + sintheta * sin(angl);   

		Mat dtheta(ds.rows,ds.cols,ds.type());
		Mat spread(ds.rows,ds.cols,ds.type());

		for (int i = 0; i < spread.rows; i++){
			for (int j = 0; j < spread.cols; j++){
				float dsValue = ds.at<float>(i,j);
				float dcValue = dc.at<float>(i,j);
				float thetaValue = abs(atan2(dsValue,dcValue));
				float spreadValue = exp(-pow(thetaValue,2) / ( 2 * pow(thetaSigma,2)));
				dtheta.at<float>(i,j) = thetaValue;
				spread.at<float>(i,j) = spreadValue;
			}
		}

		for (int s = 1; s <=nScales; s++){

			wavelength = wavelength * mult;

			double fo = 1.0/wavelength; 

			Mat logGabor(radius.rows,radius.cols,radius.type());

			Mat mul(logGabor.rows,logGabor.cols,CV_32F);

			for (int i = 0; i < logGabor.rows; i++){
				for (int j = 0; j < logGabor.cols; j++){
					float radiusValue = radius.at<float>(i,j);
					float logValue = exp((-pow(log(radiusValue/fo),2)) / (2 * pow(log(sigmaOnf),2)));
					logGabor.at<float>(i,j) = logValue;
					logGabor.at<float>(floor(rows/2)+1,floor(cols/2)+1) = 0;
					mul.at<float>(i,j) = logGabor.at<float>(i,j)*spread.at<float>(i,j);
				}
			}

			Mat zeros = Mat::zeros(mul.rows,mul.cols,mul.type());

			CvMat* tmpMul = new CvMat(mul);

			cvShiftDFT(tmpMul,tmpMul);

			Mat filter (logGabor.rows,logGabor.cols,CV_32F);

			mul.copyTo(filter);

			filters.push_back(filter);

			//wavelength = wavelength * mult;

		}

	}
}

string GaborExtractor::getName(){
	return "GaborExtractor";
}
