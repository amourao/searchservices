#include "VWBasicClassifier.h"


VWBasicClassifier::VWBasicClassifier(){

}


VWBasicClassifier::~VWBasicClassifier(){
		VW::finish(*vwTest);
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
	
	stringstream ss;
	
	ss << "-f train.model --oaa " << numberOfClasses << " --passes 1000 -c " ; 
	//ss << "
	string params = ss.str();
			
	vwModel = VW::initialize(params);
	{
		
		//cout << 'a' << dego++ << endl;
		importToVowpalFormat(trainData,trainLabels);		
		//cout << 'a' << dego++ << endl;
	}
	VW::finish(*vwModel);

		
	stringstream ss1;
	ss1 << "-t -i train.model"; 
	//ss << "
	string params1 = ss1.str();
	vwTest = VW::initialize(params1);

}

float VWBasicClassifier::classify( cv::Mat query){
	float label = 0;
	{
		
		label = importToVowpalFormatTest(query);	
	}
	return label;

}

void VWBasicClassifier::test( cv::Mat testData, cv::Mat testLabels ){
	//TODO
}
string VWBasicClassifier::getName(){
	return "VWBasicClassifier";
}



void VWBasicClassifier::importTxtToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels){
	//1 1.0 1|cedd 1:7.0 25:3.0 26:1.0 49:7.0 50:3.0 73:6.0 74:5.0 75:1.0 97:2.0 98:1.0 121:2.0 122:1.0 +
	stringstream vwData;
	vwData.setf(std::ios_base::fixed, std::ios_base::floatfield);
	vwData.precision(2);
	for(int i = 0; i < trainData.rows; i++){
	
		vwData << ((int) initLabelToVowpalLabel[trainLabels.at<float>(i,0)]) << " 1.0 " << ((int) initLabelToVowpalLabel[trainLabels.at<float>(i,0)]) << "|a ";

		for(int j = 0; j < trainData.cols; j++)
			if (trainData.at<float>(i,j) != 0)
				vwData << (j+1) << ":" <<(trainData.at<float>(i,j)) << " ";
		
		cout << vwData.str() << endl;
		const char* line = vwData.str().c_str();
		vwData.str("");
		vwData.clear();
		char* line2;
		line2 = const_cast<char *>(line);
		example* e = VW::read_example(*vwModel, line2);
		vwModel->learn(e);
		VW::finish_example(*vwModel, e);
	}
}

void VWBasicClassifier::importToVowpalFormat(cv::Mat trainData, cv::Mat trainLabels){
	{
	ezexample ex(vwModel, false);
	for(int i = 0; i < trainData.rows; i++){
	
		for(int j = 0; j < trainData.cols; j++)
			ex(vw_namespace('a'))(j+1)(trainData.at<float>(i,j));
				
		stringstream ss;
		ss << ((int) initLabelToVowpalLabel[trainLabels.at<float>(i,0)]) << " 1.0 " << i;
		//cout << ss.str() << " " << trainLabels.at<float>(i,0) << " " << initLabelToVowpalLabel[trainLabels.at<float>(i,0)] << endl;
		//getchar();
		ex.set_label(ss.str());
		ex.train();	
	}
	
	}
}

float VWBasicClassifier::importToVowpalFormatTest(cv::Mat testData){
	ezexample ex(vwTest, false);
	for(int j = 0; j < testData.cols; j++)
		ex(vw_namespace('a'))(j+1)(testData.at<float>(0,j));
				
	stringstream ss;
		
	ss << "1.0 ";
		
	ex.set_label(ss.str());
	//cout << ex.predict() << endl;
	return (float)vowpalLabelToinitLabel[ex.predict()];
}
