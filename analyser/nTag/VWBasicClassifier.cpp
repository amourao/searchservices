#include "VWBasicClassifier.h"


VWBasicClassifier::VWBasicClassifier(){
	modelName = "a";
}

VWBasicClassifier::VWBasicClassifier(string _modelName){
	modelName = _modelName;
}

VWBasicClassifier::~VWBasicClassifier(){

}

void VWBasicClassifier::shuffleTrainingData (cv::Mat& trainData, cv::Mat& trainLabels) {
	cv::Mat newTrainData;
	cv::Mat newTrainLabels;

	std::vector<int> myvector;
	for (int i=0; i<trainData.rows; i++) {
		myvector.push_back(i);
	}
	std::random_shuffle ( myvector.begin(), myvector.end() );
	
	for (int i=0; i<myvector.size(); i++) {
		newTrainData.push_back(trainData.row(myvector.at(i)));
		newTrainLabels.push_back(trainLabels.row(myvector.at(i)));
	}

	newTrainData.copyTo(trainData);
	newTrainLabels.copyTo(trainLabels);

}


void VWBasicClassifier::train( cv::Mat trainData, cv::Mat trainLabels ){
	
	shuffleTrainingData(trainData,trainLabels);

	int dego = 0;
	
	initLabelToVowpalLabel.clear();
	vowpalLabelToinitLabel.clear();

	numberOfClasses = 0;
	int uniqueCount = 1;
	for(int i = 0; i < trainLabels.rows; i++){
		float label = trainLabels.at<float>(i,0);
		if (initLabelToVowpalLabel.count(label) == 0){
			initLabelToVowpalLabel.insert(std::pair<int,int>(label,uniqueCount));
			vowpalLabelToinitLabel.insert(std::pair<int,int>(uniqueCount,label));
			uniqueCount++;
		}
	}

	numberOfClasses = uniqueCount-1;
	

	importTxtToVowpalFormat(trainData,trainLabels);

	stringstream ss;
	
	ss << "vw -d ./tmpData/" <<  modelName << ".txt -k -c -f ./tmpData/" << modelName <<".model --ect " << numberOfClasses  << " --quiet"; 
	//ss << <
	//string params = ss.str();
	std::system(ss.str().c_str());

}

float VWBasicClassifier::classify( cv::Mat query){
	exportTxtToVowpalFormatClassification(query);
	return predictFromFile();
}

void VWBasicClassifier::test( cv::Mat testData, cv::Mat testLabels ){
	//TODO
}
string VWBasicClassifier::getName(){
	return "VWBasicClassifier";
}



void VWBasicClassifier::importTxtToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels){
	//1 1.0 1|cedd 1:7.0 25:3.0 26:1.0 49:7.0 50:3.0 73:6.0 74:5.0 75:1.0 97:2.0 98:1.0 121:2.0 122:1.0 +
	stringstream ss;
	ss << "./tmpData/" + modelName + ".txt";
  	ofstream vwData(ss.str().c_str());
	vwData.setf(std::ios_base::fixed, std::ios_base::floatfield);
	vwData.precision(5);
	for(int i = 0; i < trainData.rows; i++){
		vwData << ((int) initLabelToVowpalLabel[trainLabels.at<float>(i,0)]) << " | ";

		for(int j = 0; j < trainData.cols; j++)
			if (trainData.at<float>(i,j) != 0)
				vwData << (j+1) << ":" <<(trainData.at<float>(i,j)) << " ";
		vwData << endl;
	}
	vwData.flush();
	vwData.close();
}


void VWBasicClassifier::exportTxtToVowpalFormatClassification(cv::Mat testData){

	stringstream ss; 
	ss << "./tmpData/" + modelName + ".p.txt";
  	ofstream vwData(ss.str().c_str());
	vwData.setf(std::ios_base::fixed, std::ios_base::floatfield);
	vwData.precision(5);
	vwData << "1.0 | "; 
	for(int j = 0; j < testData.cols; j++)
		if (testData.at<float>(0,j) != 0)
			vwData << (j+1) << ":" << testData.at<float>(0,j) << " ";
	vwData.flush();
	vwData.close();
}

float VWBasicClassifier::predictFromFile(){
	
	stringstream ss;

	ss << "vw -t -d ./tmpData/" << modelName <<".p.txt -i ./tmpData/" << modelName <<".model -p ./tmpData/" << modelName << ".predict --quiet"; 
	//ss << <
	//string params = ss.str();
	std::system(ss.str().c_str());
 
	float predict = 0;

	stringstream ss2;

	ss2 << "./tmpData/" << modelName <<".predict";

	ifstream prection(ss2.str().c_str());

	std::string line; 
	std::getline(prection, line);
    std::istringstream in(line);
    in >> predict;
	
	//cout << predict << " " << vowpalLabelToinitLabel[predict] << endl;
	return (float)vowpalLabelToinitLabel[predict];
}

bool VWBasicClassifier::save(string basePath){
	//TODO do the function
	return false;
}

bool VWBasicClassifier::load(string basePath){
	//TODO do the function
	return false;
}