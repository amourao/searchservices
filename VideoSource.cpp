#include "VideoSource.h"


VideoSource::VideoSource(string filename){
	capture = VideoCapture(filename);
	if(!capture.isOpened()) 
		cout << "Capture from video " << filename <<  " didn't work"<< endl;
}


VideoSource::~VideoSource(){

}

Mat VideoSource::nextImage(){
	Mat frame;
	capture >> frame;
	return frame;

}

string VideoSource::getImageInfo(){
	return "";
}

bool VideoSource::isAvailable(){
	return capture.isOpened();
}
