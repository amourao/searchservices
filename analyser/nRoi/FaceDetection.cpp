#include "FaceDetection.h"

static FaceDetection faceDetectorFactory;

FaceDetection::FaceDetection(){
	FactoryAnalyser::getInstance()->registerType("FaceDetection",this);
	FactoryAnalyser::getInstance()->registerType("FastFaceDetection",this);
}



FaceDetection::FaceDetection(string _cascadePath, string _nestedCascadePath, double _scaleChangeFactor, double _eyeScaleChangeFactor, cv::Size _minSize, cv::Size _maxSize, bool _preProcess){
	faceCascade.load( _cascadePath );
	eyesCascade.load( _nestedCascadePath );

	scaleChangeFactor = _scaleChangeFactor;
	eyeScaleChangeFactor = _eyeScaleChangeFactor;

	minSize = _minSize;
	maxSize = _maxSize;

	preProcess = _preProcess;
}

void* FaceDetection::createType(string& typeId) {

	if (typeId == "FaceDetection")
		return new FaceDetection(FACE_DETECTION_CASCADE_PATH + "haarcascade_frontalface_default.xml",FACE_DETECTION_CASCADE_PATH + "haarcascade_eye_tree_eyeglasses.xml",4,1, cv::Size(20,20),cv::Size(50,50),true);
	else if (typeId == "FastFaceDetection")
		return new FaceDetection(FACE_DETECTION_CASCADE_PATH + "haarcascade_frontalface_default.xml",FACE_DETECTION_CASCADE_PATH + "haarcascade_eye_tree_eyeglasses.xml",4,1, cv::Size(20,20),cv::Size(50,50),false);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}
FaceDetection::~FaceDetection(){

}

void FaceDetection::extractFeatures(Mat& source,
	map<string, region>& features) {

	vector<Mat> faceImages;
	vector<cv::Point> locations;
	vector<cv::Rect> faceRois;

	detectFaces(source,faceImages,locations,faceRois);

	for(unsigned int i = 0; i < faceRois.size(); i++){
		region r;
		cv::Rect roi = faceRois.at(i);
		r.x = roi.x;
		r.y = roi.y;
		r.width = roi.width;
		r.height = roi.height;

		stringstream ss;
		ss << "Face" << i;
		features[ss.str()] = r;
	}
}

void FaceDetection::detectFaces(Mat& img, vector<Mat>& faceImages, vector<cv::Point>& locations, vector<Rect>& faceRois){
	Mat imgGrayscale;
	Mat imgGrayscaleScalled( cvRound (img.rows/scaleChangeFactor ), cvRound(img.cols/scaleChangeFactor), CV_8UC1 );
	cv::Point eye1, eye2;

	cvtColor( img, imgGrayscale, CV_BGR2GRAY );

	resize( imgGrayscale, imgGrayscaleScalled, imgGrayscaleScalled.size(), 0, 0, INTER_LINEAR );
	equalizeHist( imgGrayscaleScalled, imgGrayscaleScalled );

	std::vector<Rect> facesVectorCascade;

	double t = (double)cvGetTickCount();
	faceCascade.detectMultiScale( imgGrayscaleScalled, facesVectorCascade, 1.2, 3, 0|CV_HAAR_SCALE_IMAGE,minSize,maxSize);
	t = (double)(cvGetTickCount()-t)/((double)cvGetTickFrequency());
	



	for (unsigned int q = 0; q < facesVectorCascade.size(); q++){	
		Rect r = facesVectorCascade.at(q);
		cv::Rect newR = cv::Rect(r.x,r.y,r.width,r.height);
		newR.x *= scaleChangeFactor;
		newR.y *= scaleChangeFactor;
		newR.height *= scaleChangeFactor;
		newR.width *= scaleChangeFactor;

		cv::Point center;
		center.x = cvRound((r.x + r.width*0.5)*scaleChangeFactor);
		center.y = cvRound((r.y + r.height*0.5)*scaleChangeFactor);

		Mat smallImgColorROI =  img (newR);

		if (preProcess){
			Mat preprocessed;
			cv::Rect newRoi;
			if (preProcessFaceImage(smallImgColorROI,preprocessed,newRoi)){
				smallImgColorROI = preprocessed;
				newR = newRoi;
			}

		}

		locations.push_back(center);
		faceRois.push_back(newR);
		faceImages.push_back(smallImgColorROI);
	}

}


bool FaceDetection::preProcessFaceImage(Mat& img, Mat& faceImage, cv::Rect& roi ){

	Mat imgGrayscaleScalled( cvRound (img.rows/eyeScaleChangeFactor ), cvRound(img.cols/eyeScaleChangeFactor), CV_8UC1 );
	Mat imgGrayscale;
	cvtColor( img, imgGrayscale, CV_BGR2GRAY );
	resize( imgGrayscale, imgGrayscaleScalled, imgGrayscaleScalled.size(), 0, 0, INTER_LINEAR );


	equalizeHist( imgGrayscaleScalled, imgGrayscaleScalled );
	std::vector<Rect> eyesVectorCascade;

	eyesCascade.detectMultiScale( img, eyesVectorCascade,	1.2, 2, 0
		//|CV_HAAR_FIND_BIGGEST_OBJECT
		//|CV_HAAR_DO_ROUGH_SEARCH
		//|CV_HAAR_DO_CANNY_PRUNING
		|CV_HAAR_SCALE_IMAGE
		
		,
		cv::Size(10,10),
		cv::Size(50,50)
		);

	vector<Eye> leftEyes;
	vector<Eye> rightEyes;

	cv::Point center;
	center.x = cvRound((img.cols*0.5));
	center.y = cvRound((img.rows*0.5));

	double radius = (img.cols + img.rows)*0.25;
	for (unsigned int q = 0; q < eyesVectorCascade.size(); q++){	
		Rect r = eyesVectorCascade.at(q);


		cv::Rect newR = cv::Rect(r.x,r.y,r.width,r.height);
		newR.x *= eyeScaleChangeFactor;
		newR.y *= eyeScaleChangeFactor;
		newR.height *= eyeScaleChangeFactor;
		newR.width *= eyeScaleChangeFactor;

		Eye auxEye;
		auxEye.eyeCenter.x = cvRound((r.x + r.width*0.5));
		auxEye.eyeCenter.y = cvRound((r.y + r.height*0.5));
		auxEye.eyeRadius = (r.width + r.height)*0.25*1;

		if (auxEye.eyeCenter.x < center.x  && auxEye.eyeCenter.y < center.y)
			leftEyes.push_back(auxEye);
		else if (auxEye.eyeCenter.x > center.x  && auxEye.eyeCenter.y < center.y)
			rightEyes.push_back(auxEye);


	}

	Eye leftEye;
	Eye rigthEye;

	if (!detectBestEyepair (center, radius, leftEyes, rightEyes, leftEye, rigthEye))
		return false;

	double y;
	double x;
	if(leftEye.eyeCenter.x >rigthEye.eyeCenter.x){
		y = leftEye.eyeCenter.y-rigthEye.eyeCenter.y;
		x = leftEye.eyeCenter.x-rigthEye.eyeCenter.x;
	}else{
		y = rigthEye.eyeCenter.y-leftEye.eyeCenter.y;
		x = rigthEye.eyeCenter.x-leftEye.eyeCenter.x;
	}

	double angle = atan2(y,x) * 180 / PI;

	if (abs(angle) > 15)
		return false;

	Mat rotatedImage;

	Mat rot_mat = getRotationMatrix2D(center, angle, 1.0);
	warpAffine(img, rotatedImage, rot_mat, img.size());

	rotateEye(center,leftEye.eyeCenter, angle); 
	rotateEye(center,rigthEye.eyeCenter, angle);

	roi =  computeROI(center, leftEye.eyeCenter, rigthEye.eyeCenter, radius);

	ajustROI(roi, img.size());

	faceImage=Mat(img,roi);
	resize(faceImage,faceImage,cv::Size(92,112),0,0,INTER_CUBIC);

	imshow("a",faceImage);
	waitKey(1);
	return 0;
}

bool FaceDetection::detectBestEyepair (Point& center, double faceRadius, vector<Eye>& leftEyes, vector<Eye>& rightEyes, Eye& bestLeftEye, Eye& bestRightEye){
	Eye tmpRightEye;
	bool tmpRightDetected = false;
	bool tmpLeftDetected = false;
	for (unsigned int l = 0; l < leftEyes.size(); l++){
		Eye leftEye = leftEyes.at(l);
		for (unsigned int r = 0; r < rightEyes.size(); r++){
			Eye rightEye = rightEyes.at(r);
			if(!tmpRightDetected){
				tmpRightEye = rightEye;
				tmpRightDetected = true;
			}
			else if (abs(abs(leftEye.eyeCenter.x-center.x)-abs(rightEye.eyeCenter.x-center.x)) <
				abs(abs(leftEye.eyeCenter.x-center.x)-abs(tmpRightEye.eyeCenter.x-center.x))){
					tmpRightEye = rightEye;
			}
			//cout << "l: " << leftEye->eyeCenter << " " << leftEye->eyeRadius << endl;
			//cout << "old r: " << tmpRightEye->eyeCenter << " " << tmpRightEye->eyeRadius << endl;
			//cout << "new r: " << rightEye->eyeCenter << " " << rightEye->eyeRadius << endl;
			//cout << abs(abs(leftEye->eyeCenter.x-center.x)-abs(rightEye->eyeCenter.x-center.x)) << " " << abs(abs(leftEye->eyeCenter.x-center.x)-abs(tmpRightEye->eyeCenter.x-center.x)) << endl;
			//cout << abs(leftEye->eyeRadius-rightEye->eyeRadius) << " "  << abs(leftEye->eyeRadius-tmpRightEye->eyeRadius) << endl;
		}

		if (((tmpRightDetected && !tmpLeftDetected) ||

			(abs(abs(leftEye.eyeCenter.x-center.x)-abs(tmpRightEye.eyeCenter.x-center.x)) <
			abs(abs(bestLeftEye.eyeCenter.x-center.x)-abs(bestRightEye.eyeCenter.x-center.x)))) 

			&&

			abs(tmpRightEye.eyeCenter.x-leftEye.eyeCenter.x) > faceRadius/1.6){

				tmpLeftDetected = true;
				bestLeftEye.eyeCenter = leftEye.eyeCenter;
				bestLeftEye.eyeRadius = leftEye.eyeRadius;
				bestRightEye.eyeCenter = tmpRightEye.eyeCenter;
				bestRightEye.eyeRadius = tmpRightEye.eyeRadius;
		}
	}

	return tmpLeftDetected;
}

void FaceDetection::rotateEye(cv::Point& center, cv::Point& eye, double angle){

	eye.x = eye.x - center.x;
	eye.y =  eye.y - center.y;

	//cout << "eye: " << eye.x << " " << eye.y << endl;
	//cout << "newEyeTmp: " << newEye.x << " " << newEye.y << endl;

	double h = sqrt(pow((double)eye.x,2)+pow((double)eye.y,2));
	double theta = atan2(abs((double)eye.y),abs((double)eye.x))* 180.0 / PI;
	if (eye.x < 0){
		eye.x = (int)( h*cos((-angle+theta-180.0) * PI/180.0) + center.x);
		eye.y = (int) (center.y+h*sin((-angle+theta-180.0)* PI/180.0 ));
	} else {
		eye.x = (int) (h*cos((-angle-theta) * PI/180.0) + center.x);
		eye.y = (int) (center.y+h*sin((-angle-theta) * PI/180.0));
	}

	//cout << "eye: " << eye.x << " " << eye.y << endl;
	//cout << "newEye: " << newEye.x << " " << newEye.y << endl;

}

cv::Rect FaceDetection::computeROI(cv::Point& center, cv::Point& eye1, cv::Point& eye2, int radius){
	cv::Point middleEyePoint;

	double eyeDistance =  abs(eye1.x - eye2.x);

	int width = (int) (eyeDistance * 1.8);
	int height = (int) (width *1.4);
	middleEyePoint.x = (eye1.x + eye2.x) / 2;
	middleEyePoint.y = eye1.y;
	cv::Rect roi;

	roi.x = middleEyePoint.x - width / 2;
	roi.y = middleEyePoint.y - height / 4;
	roi.width = width;
	roi.height = height;

	return roi;
}

void FaceDetection::train(string trainFile) {
	cout << "Training is not necessary for face detection.";
	//Trained files are args in the constructor
}

string FaceDetection::test(string testFile) {
	//TODO
	return "TODO";
}

string FaceDetection::crossValidate(string testFile) {
	//TODO
	return "TODO";
}

string FaceDetection::getName() {
	return "FaceDetector";
}





void FaceDetection::ajustROI (cv::Rect& roi, cv::Size s){
	if(roi.x <0)
		roi.x = 0;
	if (roi.y < 0)
		roi.y = 0;
	if (roi.x+roi.width > s.width)
		roi.width = s.width - roi.x - 1;
	if (roi.y+roi.height > s.height)
		roi.height = s.height - roi.y -1 ;
}
