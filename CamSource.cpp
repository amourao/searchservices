#include "CamSource.h"


CamSource::CamSource(){
	init(0);
}

CamSource::CamSource(int cameraIndex){
	init(cameraIndex);
}

void CamSource::init(int cameraIndex){
	capture = VideoCapture(cameraIndex);
	if(!capture.isOpened()) {
		cout <<  "Capture from cam " << cameraIndex <<  " didn't work" << endl;
	} else {
		capture.set(CV_CAP_PROP_FRAME_WIDTH,1280);
		capture.set(CV_CAP_PROP_FRAME_HEIGHT,720);
	}
}

CamSource::~CamSource(){

}

Mat CamSource::nextImage(){
	Mat frame;
	capture >> frame;
	return frame;

}

string CamSource::getImageInfo(){
	return "";
}

bool CamSource::isAvailable(){
	return capture.isOpened();
}
