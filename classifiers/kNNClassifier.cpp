#include "kNNClassifier.h"



kNNClassifier::kNNClassifier()
{
	flannIndex = new flann::Index();
}


kNNClassifier::~kNNClassifier()
{

}

void kNNClassifier::train( cv::Mat trainData, cv::Mat _trainLabels )
{
	flann::LinearIndexParams params = flann::LinearIndexParams();

	flannIndex->build(trainData,params);

	trainLabels = _trainLabels;
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

	//query.colRange(0,5);
	cv::Mat indices (1,neighboursCount,CV_32S);

	cv::Mat dists (1,neighboursCount,CV_32F);
//cout << j++ << endl;
	flannIndex->knnSearch(query,indices,dists,neighboursCount);

	vector<int> matches(8,0);

//cout << j++ << endl;

	//cout << trainLabels << endl;

	for (int i = 0; i < neighboursCount; i++){
		int imageId = indices.at<int>(0,i);
		int detectedEmotion = trainLabels.at<float>(0,imageId);
		matches[detectedEmotion]++;
	}
//cout << j++ << endl;
	float maxEmotion = -1;
	int maxEmotionCount = 0;
//cout << j++ << endl;
	for (int i = 1; i < 8; i++){
		
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
