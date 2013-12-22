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

string LireExtractor::genRandom(int len) {
    stringstream ss;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        ss << alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return ss.str();
}

void LireExtractor::extractFeatures(Mat& src, Mat& dst){
	
	stringstream ss6;
	ss6 << "./tmpData/" << genRandom(12) << ".png";
	
	string filename = ss6.str();
	
	imwrite(filename,src);
	
	stringstream filenameNew;
	
	filenameNew << filename  << "." << type << ".bin";
	
	stringstream ss;
	
	
	ss << "java -cp ./analyser/data/lire/:./analyser/data/lire/lire.jar:./analyser/data/lire/lucene-core-4.0.0.jar:./analyser/data/lire/lucene-analyzers-common-4.0.0.jar SimpleExtractor " << 
	filename << " " << type << " " << filenameNew.str();
			
	//cout << ss.str() << endl;
	std::system(ss.str().c_str());
	
	cv::Mat featuresMat;
	cv::Mat labels;
	
	std::string name = filenameNew.str();
	MatrixTools::readBin(name, featuresMat,labels);

	dst = featuresMat;
}

void LireExtractor::extractFeatures(string filename, vector<float>& features){
	stringstream filenameNew;
	
	filenameNew << filename  << "." << type << ".bin";
	
	stringstream ss;
	
	
	ss << "java -cp ./analyser/data/lire/:./analyser/data/lire/lire.jar:./analyser/data/lire/lucene-core-4.0.0.jar:./analyser/data/lire/lucene-analyzers-common-4.0.0.jar SimpleExtractor " << 
	filename << " " << type << " " << filenameNew.str();
			
	cout << ss.str() << endl;
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


