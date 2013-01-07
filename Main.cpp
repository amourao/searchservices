#include <iostream>
#include <string>
#include <sstream>

#include "classifiers/SRClassifier.h"
#include "classifiers/kNNClassifier.h"
#include "classifiers/SVMClassifier.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "sources/ImageSource.h"
#include "sources/CamSource.h"
#include "sources/SingleImageSource.h"
#include "sources/VideoSource.h"
#include "sources/TextFileSource.h"
#include "extractors/GaborExtractor.h"

#include "extractors/SURFExtractor.h"
#include "extractors/HistogramExtractor.h"
#include "extractors/SegmentedHistogramExtractor.h"

#include "extractors/FaceDetection.h"

#include "TrainTestFeaturesTools.h"


using namespace std;

void testDetection(int argc, char *argv[]){

/*
	SingleImageSource vs("./data/teste/faces.png");

	CamSource cs;


	FaceDetection fd("./data/FaceDetection/haarcascades/haarcascade_frontalface_alt.xml","./data/FaceDetection/haarcascades/haarcascade_eye_tree_eyeglasses.xml",4,1, cv::Size(20,20),cv::Size(50,50));

	vector<Mat> faces;
	vector<cv::Point> centers;
	vector<Rect> rois;


	while(vs.isAvailable()){
		Mat image = vs.nextImage();

		double t = (double)cvGetTickCount();
		fd.detectFaces(image,faces,centers,rois);
		t = (double)(cvGetTickCount()-t)/((double)cvGetTickFrequency());
		cout << t/1000.0 << " ms " << faces.size() << " faces" << endl;

		faces.clear();
		centers.clear();
		rois.clear();
	}
*/

}

void extractAllFeaturesImEmotion(string testPath, string output){


	TextFileSource is (testPath);

	//HistogramExtractor histogramExtractor (8);

	vector<cv::Rect> rectangleRois = vector<cv::Rect>();
	rectangleRois.push_back(cv::Rect(0,0,46,64));
	rectangleRois.push_back(cv::Rect(46,64,46,112-64));
	rectangleRois.push_back(cv::Rect(46,0,46,64));
	rectangleRois.push_back(cv::Rect(0,64,46,112-64));
	rectangleRois.push_back(cv::Rect(0,10,92,30));
	rectangleRois.push_back(cv::Rect(20,65,52,30)); 

	GaborExtractor faceGaborExtractor (92,112,4,6,rectangleRois);


	fstream faceGaborExtractorF (output, std::ios::out | std::ios::binary);

	//fstream faceEigenExtractorF ("eigen.bin", std::ios::out | std::ios::binary);

	//int i = 0;

	cv::Mat src;
	cv::Mat eigen;
	cv::Mat gabor;
	cv::Mat gaborNeutral;
	cv::Mat hist;

	float faceGaborExtractorS = faceGaborExtractor.getFeatureVectorSize();
	//float faceEigenExtractorS = faceEigenExtractor.getFeatureVectorSize();
	//float histogramExtractorS = histogramExtractor.getFeatureVectorSize();

	float size = is.getImageCount();

	faceGaborExtractorF.write( (const char*)& faceGaborExtractorS, sizeof(float) );
	//faceEigenExtractorF.write( (const char*)& faceEigenExtractorS, sizeof(float) );
	//histogramExtractorF.write( (const char*)& histogramExtractorS, sizeof(float) );

	faceGaborExtractorF.write( (const char*)& size, sizeof(float));
	//faceEigenExtractorF.write( (const char*)& size, sizeof(float) );
	//histogramExtractorF.write( (const char*)& size, sizeof(float) );

	int u = 0;

	double lastT = cvGetTickCount();

	while(!(src = is.nextImage()).empty()){
		//float id;
		//float detected;
		float expected;

		string idS;
		string neutralPath;
		string expectedS;



		string imageInfo = is.getImageInfo();

		stringstream liness(imageInfo);
		//getline(liness, idS, ';');
		getline(liness, expectedS, ';');
		getline(liness, neutralPath);


		//istringstream ( idS ) >> id;

		//istringstream ( detectedS ) >> detected;

		istringstream ( expectedS ) >> expected;


		
		if(!neutralPath.empty()){
			
			cv::Mat neutral = imread(neutralPath);
		//faceEigenExtractor.extractFeatures(src,eigen);
		//histogramExtractor.extractFeatures(src,hist);
		faceGaborExtractor.extractFeatures(src,gabor);
		faceGaborExtractor.extractFeatures(neutral,gaborNeutral);

		cv::Mat gaborMinusNeutral = gabor-gaborNeutral;

		normalize(gaborMinusNeutral, gaborMinusNeutral, 0,1, CV_MINMAX);
		


		gabor = gaborMinusNeutral;
		//faceEigenExtractorF.write( (const char*)& id, sizeof(float) );

		//float teste;

		//cout << "cols: " << eigen.cols << endl;
		/*
		for(int d = 0; d < eigen.cols; d++){
			teste = eigen.at<float>(0,d);
			//cout << eigen.at<float>(0,d) << endl;
			//getchar();
			faceEigenExtractorF.write( (const char*)& teste, sizeof(float));
			faceEigenExtractorF.flush();
		}
		*/
		//faceEigenExtractorF.close();
		/*
		histogramExtractorF.write( (const char*)& id, sizeof(float) );
		//histogramExtractorF.flush();
		for(int i = 0; i < hist.cols; i++){
			histogramExtractorF.write( (const char*)& hist.at<float>(0,i), sizeof(float));
			//
		}
		
		histogramExtractorF.flush();
		*/
		
		//faceGaborExtractorF.write( (const char*)& detected, sizeof(float) );
		faceGaborExtractorF.write( (const char*)& expected, sizeof(float) );

		for(int i = 0; i < gabor.cols; i++){
			faceGaborExtractorF.write( (const char*)& gabor.at<float>(0,i), sizeof(float));
		}
		
		u++;

		if(u % 250 == 0){
			cout << u << endl;
			double rem = ((size-u)/(60*250)) * ((double)cvGetTickCount() - lastT)/((double)cvGetTickFrequency()*1000000);
			cout << rem << " minutes remaining" << endl;
			lastT = cvGetTickCount();
		}

	}

	}

	//faceEigenExtractorF.close();
	faceGaborExtractorF.close();
	//histogramExtractorF.close();



}

int testSURF(int argc, char *argv[]){
	//extractAllFeaturesImEmotion(argv[1],argv[2]);
	
	FeatureExtractor* s = new SURFExtractor(1000);
	cv::Mat features;
	String file(argv[1]);
	s->extractFeatures(file,features);
	
	cout << features.rows << " " << features.cols << endl << s->getFeatureVectorSize() << endl;
	
	return 0;
}

int testSR(int argc, char *argv[]){

	FeatureExtractor* s = new SURFExtractor(1000);
	cv::Mat features;
	String file("/home/and/Code/faceframework/data/teste/fruits.jpg");

	s->extractFeatures(file,features);
	
	return 0;
}

int testEverything(int argc, char *argv[]){

	string file(argv[1]);
	TextFileSource is (file);
	
	//FeatureExtractor* s = new SegmentedHistogramExtractor(atoi(argv[3]),atoi(argv[4]),atoi(argv[5]));
	FeatureExtractor* s = new HistogramExtractor(atoi(argv[3]));

	cv::Mat src;
	cv::Mat dst;
	cv::Mat features;
	cv::Mat labels;
	int i = 0;
	for(int k = 0; k < 200/*is.getImageCount()*/; k++){
	if(!(src = is.nextImage()).empty()){
		
		cv::Mat featuresRow;
		if (src.channels() == 1){
			cv::cvtColor(src, dst, CV_GRAY2BGR);
		} else
			dst = src;
		s->extractFeatures(dst,featuresRow);
		
		cv::Mat label(1,1,CV_32F);
		
		string path,idStr;
		
		stringstream liness(is.getImageInfo());

		getline(liness, path, ';');
		getline(liness, idStr, '\r');
		
		label.at<float>(0,0) = atoi(idStr.c_str());
		if((i-1) % 50 == 0)
			cout << "a" << i << " " << is.getImageCount() << " " << path <<  endl;
		i++;
		features.push_back(featuresRow);
		labels.push_back(label);
	}
}
	IClassifier* sr = new SRClassifier();
	IClassifier* knn = new kNNClassifier();
	IClassifier* svm = new SVMClassifier();
	
	vector<IClassifier*> classi;
	classi.push_back(sr);
	classi.push_back(knn);
	classi.push_back(svm);
	
	TrainTestFeaturesTools ttft(features,labels,classi);
	
	cout << ttft.crossValidateAll(atoi(argv[2])) << endl;
	
	return 0;
}

void readFile(string file, Mat& dataR, Mat& labelsR,FeatureExtractor* s){
		cv::Mat features;
	cv::Mat labels;
		TextFileSource is (file);
		cv::Mat src;
	cv::Mat dst;
		int i = 0;
	for(int k = 0; k < is.getImageCount(); k++){
	if(!(src = is.nextImage()).empty()){
		
		cv::Mat featuresRow;
		if (src.channels() == 1){
			cv::cvtColor(src, dst, CV_GRAY2BGR);
		} else
			dst = src;
		s->extractFeatures(dst,featuresRow);
		
		cv::Mat label(1,1,CV_32F);
		
		string path,idStr;
		
		stringstream liness(is.getImageInfo());

		getline(liness, path, ';');
		getline(liness, idStr, '\r');
		
		label.at<float>(0,0) = atoi(idStr.c_str());
		//if((i+1) % 50 == 0)
		//	cout << "a" << i << " " << is.getImageCount() << " " << path <<  endl;
		//i++;
		features.push_back(featuresRow);
		labels.push_back(label);
	} else {
		string path,idStr;
		
		stringstream liness(is.getImageInfo());

		getline(liness, path, ';');
		getline(liness, idStr, '\r');
		cout << "WARNING: error reading image " << path << endl;
	}
}
features.copyTo(dataR);
labels.copyTo(labelsR);

//cout << dataR.rows << " " << dataR.cols << endl;
}

int testINRIA(int argc, char *argv[]){

	string trainfile(argv[1]);
	string testfile(argv[2]);

	vector<FeatureExtractor*> featureExtractors;
	featureExtractors.push_back(new SegmentedHistogramExtractor(atoi(argv[4]),atoi(argv[5]),atoi(argv[6])));
	featureExtractors.push_back(new HistogramExtractor(atoi(argv[4])));

	for (unsigned int i = 0; i < featureExtractors.size(); i ++){
		if (i == 0)
			cout << "SegmentedHistogramExtractor" << endl;
		else if (i == 1)
			cout << "HistogramExtractor" << endl;
		else
			cout << i << "th Extractor" << endl;
		cv::Mat features;
		cv::Mat labels;

		cv::Mat featuresTest;
		cv::Mat labelsTest;

		readFile(trainfile,features,labels,featureExtractors.at(i));
		readFile(testfile,featuresTest,labelsTest,featureExtractors.at(i));

		vector<IClassifier*> classi;
		classi.push_back(new SRClassifier());
		classi.push_back(new kNNClassifier());
		//classi.push_back(new SVMClassifier());
	
		TrainTestFeaturesTools ttft(features,labels,featuresTest,labelsTest,classi);
	
		cout << ttft.testAll() << endl;	
	}
	return 0;
}

int main(int argc, char *argv[]){
	
	
	
	
	testINRIA(argc, argv);
	
	getchar();
	return 0;
}
