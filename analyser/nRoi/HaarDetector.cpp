#include "HaarDetector.h"

static HaarDetector haarDetectorFactory;

HaarDetector::HaarDetector(){
	FactoryAnalyser::getInstance()->registerType("haarDetector",this);
}


HaarDetector::HaarDetector(string& typeId) {
    type = typeId;
}

HaarDetector::HaarDetector(string& typeId, map<string, string>& params) {
    type = typeId;

    if (params.size() == 0)
        return;

    string _objectType = params["objectType"];
    string _cascadePath = params["cascadePath"];
    double _scaleChangeFactor = atof(params["scaleChangeFactor"].c_str());

    int minSizeW = atoi(params["minSizeW"].c_str());
    int minSizeH = atoi(params["minSizeH"].c_str());
    int maxSizeW = atoi(params["maxSizeW"].c_str());
    int maxSizeH = atoi(params["maxSizeH"].c_str());

    cv::Size _minSize(minSizeW,minSizeH);
    cv::Size _maxSize(maxSizeW,maxSizeH);

    init(_objectType,_cascadePath,_scaleChangeFactor,_minSize,_maxSize);
}

HaarDetector::HaarDetector(string _objectType,string _cascadePath, double _scaleChangeFactor, cv::Size _minSize, cv::Size _maxSize){
    init(_objectType,_cascadePath,_scaleChangeFactor,_minSize,_maxSize);
}

void HaarDetector::init(string _objectType,string _cascadePath, double _scaleChangeFactor, cv::Size _minSize, cv::Size _maxSize){
	cascade.load(CASCADE_PATH + _cascadePath );
	scaleChangeFactor = _scaleChangeFactor;
	minSize = _minSize;
	maxSize = _maxSize;
	objectType = _objectType;
}

void* HaarDetector::createType(string& typeId, map<string,string>& params) {
    return new HaarDetector(typeId,params);
}

void* HaarDetector::createType(string& typeId) {
	if (typeId == "haarDetector")
		return new HaarDetector(typeId);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

HaarDetector::~HaarDetector(){

}

void HaarDetector::extractFeatures(Mat& source,
	map<string, region>& features) {

	vector<Mat> faceImages;
	vector<cv::Point> locations;
	vector<cv::Rect> faceRois;

	detectObjects(source,faceImages,locations,faceRois);

	for(unsigned int i = 0; i < faceRois.size(); i++){
		region r;
		cv::Rect roi = faceRois.at(i);
		r.x = roi.x;
		r.y = roi.y;
		r.width = roi.width;
		r.height = roi.height;
		r.annotationType = objectType;

		stringstream ss;
		ss << objectType << " " << i;
		features[ss.str()] = r;
	}
}

void HaarDetector::detectObjects(Mat& img, vector<Mat>& faceImages, vector<cv::Point>& locations, vector<Rect>& faceRois){
	Mat imgGrayscale;
	Mat imgGrayscaleScalled( cvRound (img.rows/scaleChangeFactor ), cvRound(img.cols/scaleChangeFactor), CV_8UC1 );

	cvtColor( img, imgGrayscale, CV_BGR2GRAY );

	resize( imgGrayscale, imgGrayscaleScalled, imgGrayscaleScalled.size(), 0, 0, INTER_LINEAR );
	equalizeHist( imgGrayscaleScalled, imgGrayscaleScalled );

	std::vector<Rect> facesVectorCascade;

	cascade.detectMultiScale( imgGrayscaleScalled, facesVectorCascade, 1.1, 3, 0|CV_HAAR_SCALE_IMAGE,minSize,maxSize);

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

		locations.push_back(center);
		faceRois.push_back(newR);
		faceImages.push_back(smallImgColorROI);
	}

}

void HaarDetector::train(string trainFile) {
	cout << "Training is not necessary for face detection.";
	//Trained files are args in the constructor
}

string HaarDetector::test(string testFile) {
	//TODO
	return "TODO";
}

string HaarDetector::crossValidate(string testFile) {
	//TODO
	return "TODO";
}

string HaarDetector::getName() {
	return type;
	//return "FaceDetector";
}
