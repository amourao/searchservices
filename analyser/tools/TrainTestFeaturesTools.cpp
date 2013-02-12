#include "TrainTestFeaturesTools.h"

TrainTestFeaturesTools::TrainTestFeaturesTools(cv::Mat _data, cv::Mat _labels, vector<IClassifier*> _classifiers){
	data = _data;
	labels = _labels.col(0);
	
	classifiers = _classifiers;
}

TrainTestFeaturesTools::TrainTestFeaturesTools(cv::Mat _data, cv::Mat _labels, cv::Mat _dataTest, cv::Mat _labelsTest, vector<IClassifier*> _classifiers){
	data = _data;
	labels = _labels.col(0);
	
	classifiers = _classifiers;
	
	dataTest = _dataTest;
	labelsTest = _labelsTest.col(0);

}

TrainTestFeaturesTools::~TrainTestFeaturesTools(){
		
}
	
Mat TrainTestFeaturesTools::getTrainingData(){
	return data;
}

Mat TrainTestFeaturesTools::getTestData(){
	return dataTest;
}

Mat TrainTestFeaturesTools::getTrainingLabels(){
	return labels;
}

Mat TrainTestFeaturesTools::getTestLabels(){
	return labelsTest;
}

string TrainTestFeaturesTools::crossValidateAll(int numberOfDivisions){
	stringstream ss;
		
	double* min = new double();
	double* max = new double();
	
	int* minInd  = new int();
	int* maxInd  = new int();
	
	//cout << labels << endl;
	cv::minMaxIdx(labels.col(0), min, max, minInd, maxInd, Mat());
		
	int numberOfClasses = (*max)+1;
	
	delete min;
	delete max;
	delete minInd;
	delete maxInd;

	ss << "Validate all" << endl << endl;
	for(unsigned int i = 0; i < classifiers.size(); i++){
		vector<int> correctGuesses(numberOfClasses,0);
		vector<int> falsePositives(numberOfClasses,0);
	
		Mat confusionMatrix= Mat::zeros(numberOfClasses,numberOfClasses,CV_32F);		
		
		ss << classifiers.at(i)->getName() << endl;
		for(int j = 0; j < numberOfDivisions; j++){
			Mat newTrainData, newTrainLabels, testData, testLabels;
			divideByClass(data,labels,numberOfDivisions,j,newTrainData,newTrainLabels,testData,testLabels);
			
			test(newTrainData,newTrainLabels,testData,testLabels,classifiers.at(i), correctGuesses, falsePositives, confusionMatrix);
		}
		ss << resultsToString(correctGuesses, falsePositives, confusionMatrix) << endl;
	}
	cout << ss.str() << endl;
	return ss.str();
}

string TrainTestFeaturesTools::testAll(){
	
	double min, max;
	int minInd, maxInd;
	cv::minMaxIdx(labels, &min, &max, &minInd, &maxInd, Mat());
		
	int numberOfClasses = max+1;
	
	stringstream ss;
	ss << "Test all" << endl << endl;
	for(unsigned int i = 0; i < classifiers.size(); i++){
		ss << classifiers.at(i)->getName() << endl;
		
		vector<int> correctGuesses(numberOfClasses,0);
		vector<int> falsePositives(numberOfClasses,0);
	
		Mat confusionMatrix = Mat::zeros(numberOfClasses,numberOfClasses,CV_32F);
				
		test(data,labels,dataTest,labelsTest,classifiers.at(i), correctGuesses, falsePositives, confusionMatrix);
		
		ss << resultsToString(correctGuesses, falsePositives, confusionMatrix) << endl;
	}
	return ss.str();
}

string TrainTestFeaturesTools::crossValidate(string name,int numberOfDivisions){
	//TODO
	return "";
}


string TrainTestFeaturesTools::test(string name){
	//TODO
	return "";
}

void TrainTestFeaturesTools::test(Mat trainData, Mat trainLabels, Mat testData, Mat testLabels, IClassifier* classifier, vector<int>& correctGuesses, vector<int>& falsePositives, Mat& confusionMatrix){
	classifier->train(trainData,trainLabels);
	for(int i = 0; i < testData.rows; i++){
		//cout << "correct label: " << (int)testLabels.at<float>(i,0) << endl;
		float label = classifier->classify(testData.row(i));
		if(label == testLabels.at<float>(i,0))
			correctGuesses.at(testLabels.at<float>(i,0))++;
		else
			falsePositives.at(testLabels.at<float>(i,0))++;
			//cout << label << " " << testLabels.at<float>(i,0) << endl;
		confusionMatrix.at<float>(testLabels.at<float>(i,0),label)++;	
	}
}

IClassifier* TrainTestFeaturesTools::getClassifier(string name){
	//TODO
	return (IClassifier*)-1;
}



void TrainTestFeaturesTools::divideByClass(Mat trainData, Mat trainLabels, double numberOfDivisions, int currentDivision,Mat& newTrainData,Mat& newTrainLabels, Mat& testData, Mat& testLabels){
		
		
	double ratio = 1.0/numberOfDivisions;
	double min, max;
	int minInd, maxInd;
	cv::minMaxIdx(trainLabels, &min, &max, &minInd, &maxInd, Mat());
	
	int numberOfClasses = max+1;
	vector<Mat> divisionByClass(numberOfClasses);
	for(int j = 0; j < trainLabels.rows; j++){
		float label = trainLabels.at<float>(j,0);
		divisionByClass.at(label).push_back(trainData.row(j));
	}
	
	for(int i = 0; i < numberOfClasses; i++){
		
		Mat data = divisionByClass.at(i);
		if(!data.empty()){
			//TODO check if division is correct
			int divisionSize = ratio * data.rows;
		
			Mat label(1,1,CV_32F);
			label.at<float>(0,0) = i;
			
			//cout << endl << "train starts at 0 and ends at " << (currentDivision)*divisionSize << endl;
			for(int j = 0; j < (currentDivision)*divisionSize; j++){
				newTrainData.push_back(data.row(j));
				newTrainLabels.push_back(label);
			}
		
			//cout << "test starts at " <<  currentDivision*divisionSize << " and ends at " << ((currentDivision+1)*divisionSize) << " or " << data.rows << endl;
			for(int j = currentDivision*divisionSize; j < ((currentDivision+1)*divisionSize) && (j < data.rows); j++){
				testData.push_back(data.row(j));
				testLabels.push_back(label);
			}
		
			//cout << "train starts at " <<  (currentDivision+1)*divisionSize << " and ends at " << data.rows << endl;
			for(int j = (currentDivision+1)*divisionSize; j < data.rows; j++){
				newTrainData.push_back(data.row(j));
				newTrainLabels.push_back(label);
			}
			//cout << "train: " << newTrainData.rows << " test: " << testData.rows << endl;
		}
		
	}
	
}

string TrainTestFeaturesTools::resultsToString(vector<int>& correctGuesses, vector<int>& falsePositives, Mat& confusionMatrix){
	stringstream ss;
	int tp = 0;
	int fp = 0;
	for(unsigned int i = 0; i < correctGuesses.size(); i++){
		ss << correctGuesses.at(i) << ";";
		tp += correctGuesses.at(i);
	}
	ss << endl;
	for(unsigned int i = 0; i < falsePositives.size(); i++){
		ss << falsePositives.at(i) << ";";
		fp += falsePositives.at(i);
	}
	ss << endl << endl;
		
	for(int i = 0; i < confusionMatrix.rows; i++){
		for(int j = 0; j < confusionMatrix.cols; j++){
			ss << confusionMatrix.at<float>(i,j) << ";";
		}
		ss << endl;
	}
	
	ss << "Accuracy: " << tp / (double)(fp+tp) << endl;
	ss << endl;
	return ss.str();
}

void TrainTestFeaturesTools::splitDataForTest(double ratio){

	Mat newTrainData, newTrainLabels, newTestData, newTestLabels;
	divideByClass(data,labels,1.0/ratio,0,newTrainData,newTrainLabels,newTestData,newTestLabels);
	
	data = newTrainData;
	labels = newTrainLabels;
	
	dataTest = newTestData;
	labelsTest = newTestLabels;
}
