#include "EigenExtractor.h"

static EigenExtractor eigenExtractorFactory;

EigenExtractor::EigenExtractor(){
	FactoryAnalyser::getInstance()->registerType("EigenAnalyser",this);
}

EigenExtractor::EigenExtractor(string& _type){
    type = _type;
}

EigenExtractor::EigenExtractor(string& _type, map<string,string>& params){
    type = _type;

    if (params.size() > 0){
        createFaceSpace(params["faceSpace"]);
        eigenfacesCount = atoi(params["faceSpace"].c_str());
	}
}

EigenExtractor::~EigenExtractor(){
}

void* EigenExtractor::createType(string& type){
	if (type == "EigenAnalyser")
		return new EigenExtractor(type);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void* EigenExtractor::createType(string& type, map<string,string>& params){
    return new EigenExtractor(type,params);
}

void EigenExtractor::createFaceSpace(string trainDataFile){

	Mat imgVectors = readFile(trainDataFile);
	//toGrayscale(imgVectors,imgVectors);
	int numSamples = imgVectors.rows;
	int numDimensions = imgVectors.cols;

	std_deviation = 0;
	mean = 0;
	max = 0;
	min = 0;

	for (int i = 0; i < numSamples; i++){
		for (int j = 0; j < numDimensions; j++){
			mean += imgVectors.at<float>(i,j);
		}
		mean /= numDimensions;
		for (int j = 0; j < numDimensions; j++){
			std_deviation += pow(imgVectors.at<float>(i,j)-mean,2.0f);
		}
		std_deviation = pow((std_deviation/numDimensions),0.5f);
		for (int j = 0; j < numDimensions; j++){
			imgVectors.at<float>(i,j) = ((imgVectors.at<float>(i,j)-mean)/std_deviation);
		}
	}

	pca = PCA(imgVectors, Mat(),CV_PCA_DATA_AS_ROW, eigenfacesCount);
}

Mat EigenExtractor::readFile(string trainDataFile){
	Mat result;
	string line,path,id;
	ifstream myfile (trainDataFile.c_str());

	//int maxCount = 100;
	if (myfile.is_open()){
		getline (myfile,line);
		while (myfile.good()){
			getline (myfile,line);

			stringstream liness(line);
			getline(liness, path, ';');
			getline(liness, id, '\r');

			Mat image = imread(path,0);
			Mat tmpImage;
			image.convertTo(tmpImage, CV_32FC1);
			Mat transposed = tmpImage.reshape(1,1);

			result.push_back(transposed);
		}
		myfile.close();
	}

	return result;
}

void EigenExtractor::load(string xmlDataFile){

}
void EigenExtractor::save(string xmlDataFile){
}

void EigenExtractor::extractFeatures(Mat& src, Mat& dst){
	Mat src2;
	Mat tmpImage;
	cvtColor( src, src2, CV_RGB2GRAY );
	src2 = (src2-mean)/std_deviation;
	src2.convertTo(tmpImage, CV_32FC1);
	Mat transposed = tmpImage.reshape(1,1);
	pca.project(transposed,dst);
}

int EigenExtractor::getFeatureVectorSize(){
	return eigenfacesCount;
}

string EigenExtractor::getName(){
	return "EigenExtractor";
}
