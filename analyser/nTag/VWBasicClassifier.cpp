#include "VWBasicClassifier.h"


static VWBasicClassifier VWBasicClassifierFactory;

VWBasicClassifier::VWBasicClassifier(){
	FactoryClassifier::getInstance()->registerType("VWBasicClassifier",this);
}

void* VWBasicClassifier::createType(string& type){
	if (type == "VWBasicClassifier")
		return new VWBasicClassifier("");
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
	
}

VWBasicClassifier::VWBasicClassifier(string _modelName){
	if (_modelName == "")
		modelName = "./tmpData/vwTmpModel";
	else
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
	
	ss << "vw -d " <<  modelName << TRAINDATA_EXTENSION_VW << " -k -c -f " << modelName << MODEL_EXTENSION_VW <<" --oaa " << numberOfClasses  << " --passes 1000 --quiet"; 
	std::system(ss.str().c_str());

}


float VWBasicClassifier::classify( cv::Mat query){
	return predictFromFile(query);
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
	ss << modelName << TRAINDATA_EXTENSION_VW;
	
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

float VWBasicClassifier::predictFromFile(cv::Mat testData){
	string randomAppend = StringTools::genRandom(BASIC_VW_RANDOM_SIZE);

	stringstream ss1;
	ss1 << modelName << "." << randomAppend << PREDICTION_EXTENSION_VW;
	
  	ofstream vwData(ss1.str().c_str());
	vwData.setf(std::ios_base::fixed, std::ios_base::floatfield);
	vwData.precision(5);
	vwData << "1.0 | "; 
	for(int j = 0; j < testData.cols; j++)
		if (testData.at<float>(0,j) != 0)
			vwData << (j+1) << ":" << testData.at<float>(0,j) << " ";
	
	vwData.flush();
	vwData.close();

	stringstream ss;

	//ss << "echo \"" << vwData.str() << "\" | vw -t -i " << modelName << MODEL_EXTENSION_VW <<" -p " << modelName << "." << randomAppend << PREDICTION_READ_EXTENSION_VW <<" --quiet"; 
	ss << "vw -t -i " << modelName << MODEL_EXTENSION_VW << " " << ss1.str() << " -p " << modelName << "." << randomAppend << PREDICTION_READ_EXTENSION_VW << " --quiet"; 
	//ss << <
	//string params = ss.str();
	float predict = 0;
	stringstream ss2;
	ss2 << modelName << "." << randomAppend << PREDICTION_READ_EXTENSION_VW;

	std::system(ss.str().c_str());
 
	ifstream prection(ss2.str().c_str());

	std::string line; 
	std::getline(prection, line);
    std::istringstream in(line);
    in >> predict;

	return (float)vowpalLabelToinitLabel[predict];
}

bool VWBasicClassifier::save(string basePath){
	stringstream ssO;
	stringstream ssD;

	ssO << modelName << MODEL_EXTENSION_VW;

	ssD << CLASSIFIER_BASE_SAVE_PATH << basePath << MODEL_EXTENSION_VW;

	std::ifstream  src(ssO.str().c_str(), std::ios::binary);
    std::ofstream  dst(ssD.str().c_str(), std::ios::binary);

    dst << src.rdbuf();

	saveLabelMap(basePath);
    //update the model to use the new dir to save and load stuff
	load(basePath);

	return true;
}

bool VWBasicClassifier::load(string basePath){
	stringstream ssD;
	ssD << CLASSIFIER_BASE_SAVE_PATH << basePath;

	modelName = ssD.str();

	loadLabelMap(basePath);

	return true;
}

void VWBasicClassifier::loadLabelMap(string basePath){
	stringstream ssD;
	ssD << CLASSIFIER_BASE_SAVE_PATH << basePath << LABEL_FILE_EXTENSION_VW;
    
    initLabelToVowpalLabel.clear();
	vowpalLabelToinitLabel.clear();

	ifstream file(ssD.str().c_str(), ifstream::in);
	string line, path, numberOfClassesStr,id1,id2;
	
	getline(file, line);
	stringstream liness(line);
	getline(liness, numberOfClassesStr);	
	numberOfClasses = atoi(numberOfClassesStr.c_str());
	//int i = 0;
	while (getline(file, line)) {
		stringstream liness(line);
		getline(liness, id1, ',');
		getline(liness, id2);

		int label = atoi(id1.c_str());
		int vwLabel = atoi(id2.c_str());

		initLabelToVowpalLabel.insert(std::pair<int,int>(label,vwLabel));
		vowpalLabelToinitLabel.insert(std::pair<int,int>(vwLabel,label));

	}
}
void VWBasicClassifier::saveLabelMap(string basePath){
	std::map<int,int>::iterator iter;
	stringstream ssD;
	ssD << CLASSIFIER_BASE_SAVE_PATH << basePath << LABEL_FILE_EXTENSION_VW;
    ofstream labelData(ssD.str().c_str());

    labelData << numberOfClasses << endl;
    for (iter = initLabelToVowpalLabel.begin(); iter != initLabelToVowpalLabel.end(); ++iter) {
    	labelData << iter->first << "," << iter->second << endl;
    }
	labelData.close();
}