#include "SingleImageSource.h"


SingleImageSource::SingleImageSource(string filename){
	image = imread(filename, CV_LOAD_IMAGE_COLOR);
}

SingleImageSource::~SingleImageSource(){

}

Mat SingleImageSource::nextImage(){
	return image;

}

string SingleImageSource::getImageInfo(){
	return "";
}

bool SingleImageSource::isAvailable(){
	return true;
}