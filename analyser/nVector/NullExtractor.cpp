#include "NullExtractor.h"


NullExtractor::NullExtractor(void){

}

void* NullExtractor::createType(string& type){
	//TODO
	if (type == "NullExtractor")
		return new NullExtractor();
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}



NullExtractor::~NullExtractor(){ 
}

void NullExtractor::extractFeatures(Mat& src, Mat& dst){
	src.copyTo(dst);
	dst = dst.reshape(1,1);
}

int NullExtractor::getFeatureVectorSize(){
	return -1;
}

string NullExtractor::getName(){
	return "NullExtractor";
}
