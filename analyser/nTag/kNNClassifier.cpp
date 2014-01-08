#include "kNNClassifier.h"



kNNClassifier::kNNClassifier()
{
	flannIndex = NULL;
}


kNNClassifier::~kNNClassifier()
{

}

void kNNClassifier::train( cv::Mat _trainData, cv::Mat _trainLabels )
{
	
	double min, max;
	int minInd, maxInd;
	cv::minMaxIdx(_trainLabels, &min, &max, &minInd, &maxInd, Mat());
		
	numberOfClasses = max+1;
	
	flann::LinearIndexParams params = flann::LinearIndexParams();

	if ( flannIndex != NULL)
		delete flannIndex;
	flannIndex = new flann::Index();
	
	flannIndex->build(_trainData,params);

	trainLabels = _trainLabels;
	trainData = _trainData;
}
	


void kNNClassifier::test( cv::Mat testData, cv::Mat testLabels )
{

}
float kNNClassifier::classify( cv::Mat query){
	return classify(query,1);
}

string kNNClassifier::getName(){
	return "kNN";
}

float kNNClassifier::classify( cv::Mat query, int neighboursCount )
{


	//int j = 0;

	vector<int> indices (neighboursCount);
	vector<float> dists (neighboursCount);
//cout << j++ << endl;
	flannIndex->knnSearch(query,indices,dists,neighboursCount);

	vector<int> matches(numberOfClasses,0);

//cout << j++ << endl;

	//cout << trainLabels << endl;

	for (int i = 0; i < neighboursCount; i++){
		int imageId = indices.at(i);
		int detectedEmotion = trainLabels.at<float>(imageId,0);
		//cout << detectedEmotion << " " << dists.at(i)  << endl;
		//cout << query.colRange(0,5) << endl;
		//cout << trainData.row(imageId).colRange(0,5) << endl;
		//cout << norm(query,trainData.row(imageId)) << endl;
		//cout << imageId << endl;
		matches[detectedEmotion]++;
	}
//cout << j++ << endl;
	float maxEmotion = -1;
	int maxEmotionCount = 0;
//cout << j++ << endl;
	for (int i = 0; i < numberOfClasses; i++){
		
		if (matches[i] > maxEmotionCount){
			maxEmotion = i;
			maxEmotionCount = matches[i];
		}
	}
	//cout << maxEmotion << endl;
//cout << j++ << endl;

//query.colRange(0,5);
	return maxEmotion;
}

bool kNNClassifier::save(string basePath){
	//TODO do the function
	return false;
}

bool kNNClassifier::load(string basePath){
	//TODO do the function
	return false;
}