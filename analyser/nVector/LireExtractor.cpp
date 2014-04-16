#include "LireExtractor.h"

static LireExtractor lireExtractorFactory;

LireExtractor::LireExtractor(){
	FactoryAnalyser::getInstance()->registerType("lireExtractor",this);
}


LireExtractor::LireExtractor(string& _type){
	type = _type;
}

LireExtractor::LireExtractor(string& _type, map<string, string>& params){
	type = _type;

    if (params.size() == 0)
        return;

    algorithm = params["algorithm"];
}

LireExtractor::~LireExtractor(){

}


void* LireExtractor::createType(string& type){
	if (type == "lireExtractor"){
		return new LireExtractor(type);
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* LireExtractor::createType(string& type, map<string ,string>& params){
	return new LireExtractor(type,params);
}


void LireExtractor::extractFeatures(Mat& src, Mat& dst){

	stringstream ss6;
	ss6 << "./tmpData/" << StringTools::genRandom(12) << ".png";

	string filename = ss6.str();

	imwrite(filename,src);

	stringstream filenameNew;

	filenameNew << filename  << "." << algorithm << ".bin";

	stringstream ss;


	ss << "java -cp ./analyser/data/lire/:./analyser/data/lire/lire.jar:./analyser/data/lire/lucene-core-4.0.0.jar:./analyser/data/lire/lucene-analyzers-common-4.0.0.jar SimpleExtractor \"" <<
	filename << "\" " << algorithm << " \"" << filenameNew.str() << "\"";

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

	filenameNew << filename  << "." << algorithm << ".bin";

	stringstream ss;


	ss << "java -cp ./analyser/data/lire/:./analyser/data/lire/lire.jar:./analyser/data/lire/lucene-core-4.0.0.jar:./analyser/data/lire/lucene-analyzers-common-4.0.0.jar SimpleExtractor \"" <<
	filename << "\" " << algorithm << " \"" << filenameNew.str() << "\"";

	//cout << ss.str() << endl;
	std::system(ss.str().c_str());

	cv::Mat featuresMat;
	cv::Mat labels;

	std::string name = filenameNew.str();
	MatrixTools::readBin(name, featuresMat,labels);
	MatrixTools::matToVector(featuresMat, features);

}


int LireExtractor::getFeatureVectorSize(){
	if( algorithm == "cedd")
		return 144;
	else if( algorithm == "fcth")
		return 192;
	return -1;
}

string LireExtractor::getName(){
	return type;
}


