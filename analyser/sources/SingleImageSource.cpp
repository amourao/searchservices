#include "SingleImageSource.h"


SingleImageSource::SingleImageSource(std::string filename){
	image = imread(filename, CV_LOAD_IMAGE_COLOR);
}

SingleImageSource::~SingleImageSource(){

}

cv::Mat SingleImageSource::nextImage(){
	return image;

}

std::string SingleImageSource::getImageInfo(){
	return "";
}

bool SingleImageSource::isAvailable(){
	return true;
}
	int SingleImageSource::getImageCount(){
		return -1;
	}
	
	int SingleImageSource::getRemainingImageCount(){
		return -1;
	}
