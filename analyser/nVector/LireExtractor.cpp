#include "LireExtractor.h"

static LireExtractor lireExtractorFactory;

LireExtractor::LireExtractor(){
	FactoryAnalyser::getInstance()->registerType("cedd",this);
	FactoryAnalyser::getInstance()->registerType("fcth",this);
}

void* LireExtractor::createType(string& type){
	//TODO
	if (type == "cedd"){
		return new LireExtractor(type);
	}else if (type == "fcth"){
		return new LireExtractor(type);
	}cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

LireExtractor::LireExtractor(string _type){
	type = _type;
}



LireExtractor::~LireExtractor(){

}

void LireExtractor::extractFeatures(Mat& src, Mat& dst){
	//not necessary
}

void LireExtractor::extractFeatures(string filename, vector<float>& features){
	stringstream filenameNew;
	
	filenameNew << filename  << "." << type << ".bin";
	
	stringstream ss;
	
	
	ss << "java -cp /home/amourao/code/lire/:/home/amourao/code/lire/lire.jar:/home/amourao/code/lire/lucene-core-4.0.0.jar:/home/amourao/code/lire/lucene-analyzers-common-4.0.0.jar SimpleExtractor " << 
	filename << " " << type << " " << filenameNew.str();
			
	std::system(ss.str().c_str());
	
	cv::Mat featuresMat;
	cv::Mat labels;
	
	std::string name = filenameNew.str();
	MatrixTools::readBin(name, featuresMat,labels);
	MatrixTools::matToVector(featuresMat, features);

}


int LireExtractor::getFeatureVectorSize(){
	if( type == "cedd")
		return 144;
	else if( type == "fcth")
		return 192;
	return -1;
}

string LireExtractor::getName(){
	return "LireExtractor";
}


