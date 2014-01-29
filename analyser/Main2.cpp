#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <opencv2/features2d/features2d.hpp>

//#include <math>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "nTag/SRClassifier.h"

#include "sources/ImageSource.h"
#include "sources/CamSource.h"
#include "sources/SingleImageSource.h"
#include "sources/VideoSource.h"
#include "sources/TextFileSource.h"
#include "sources/TextFileSourceV2.h"
#include "nVector/GaborExtractor.h"
#include "nKeypoint/SURFExtractor.h"
#include "nVector/HistogramExtractor.h"
#include "nVector/SegmentedHistogramExtractor.h"

#include "nRoi/FaceDetection.h"

#include "nTag/kNNClassifier.h"
#include "nTag/SVMClassifier.h"
#include "nTag/VWBasicClassifier.h"

#include "tools/TrainTestFeaturesTools.h"
#include "tools/MIRFlickrImporter.h"

#include "FactoryAnalyser.h"

#include "../indexer/FactoryIndexer.h"
#include "../indexer/IIndexer.h"
#include "../indexer/FlannkNNIndexer.h"
#include "../indexer/MSIDXIndexer.h"



using namespace std;


struct tagError {
     double error;
     string tag;
 };

class  CompareTags{
     public:
     bool operator()(tagError& t1, tagError& t2){
        return t1.error > t2.error;
     }
 };

int testAllClassifiers(int argc, char *argv[]) {

	// /localstore/example.txt
	//"ImageCLEF 2012 Training Set/DVDM/IJD-54-150-g002.jpg";IJD-54-150-g002;DVDM;16
	
	if(argc < 4){
		cout << "usage: analyser <trainTextFile> <nrOfDivisions> <classifierFactoryName>"  << endl;
		cout << "nrOfDivisions == 9 leads to crossvalidation: 9 parts for train, 1 for test"  << endl;
		exit(1);
	}
	
	//Text file with images
	string file(argv[1]);
	int numberOfDivisions = atoi(argv[2]);

	string className = string(argv[3]);

	TextFileSource is(file);

	FactoryAnalyser * f = FactoryAnalyser::getInstance();
	IAnalyser* extractor = (IAnalyser*)f->createType(className);
	


	cv::Mat src;
	cv::Mat dst;
	cv::Mat features;
	cv::Mat labels;
	int i = 0;

	cout << is.getImageCount() << endl;
	for (int k = 0; k < is.getImageCount(); k++) {
		if (!(src = is.nextImage()).empty()) { // src contains the image, but the IAnalyser interface needs a path 
			cv::Mat featuresRow;


			cv::Mat label(1, 1, CV_32F);

			//parse the label from the info in the txt file
			string path, idStr, tmp1, tmp2;

			stringstream liness(is.getImageInfo());

			getline(liness, path, ';');
			getline(liness, tmp1, ';');
			getline(liness, tmp2, ';');
			getline(liness, idStr, '\r');

			label.at<float>(0, 0) = atoi(idStr.c_str());

			i++;
			
			
			//cout << path << endl;
			IDataModel* data = extractor->getFeatures(path);
			vector<float>* v = (vector<float>*) data->getValue();
			vector<float> v2 = *v;
			MatrixTools::vectorToMat(v2, featuresRow);
			
			//cout << featuresRow << endl;
			// add the features to the main feature and label matrix
			features.push_back(featuresRow);
			labels.push_back(label);
		}
	}

	// features must be in 32F (float) for the classifiers
	features.convertTo(features,CV_32F);
	labels.convertTo(labels,CV_32F);

	//prepare the classifiers
	//IClassifier* sr = new SRClassifier();
	IClassifier* knn = new kNNClassifier();
	IClassifier* svm = new SVMClassifier();

	vector<IClassifier*> classi;
	//classi.push_back(sr);
	classi.push_back(knn);
	classi.push_back(svm);

	//add to the tools
	TrainTestFeaturesTools ttft(features, labels, classi);

	//print the results
	cout << ttft.crossValidateAll(numberOfDivisions) << endl;

	return 0;
}

int testAllClassifiersBin(int argc, char *argv[]) {


	if(argc < 3){
		cout << "usage: analyser <trainBinFile> <testBinFile> ";
		cout << "OR analyser <allDataBinFile> <nrOfDivisions>"  << endl;
		cout << "nrOfDivisions == 9 leads to crossvalidation: 9 parts for train, 1 for test"  << endl;
		exit(1);
	}

	
	int divisions =  atoi(argv[2]);

	if (isdigit(argv[2][0])){ //crossvalidation
		
		string file(argv[1]);

		Mat features;
		Mat labels;
	
		MatrixTools::readBin(file, features, labels);
		
		vector<IClassifier*> classi; //choose the classifiers to test
		//classi.push_back(new SRClassifier())
		//classi.push_back(new SVMClassifier());
		classi.push_back(new kNNClassifier());
		classi.push_back(new VWBasicClassifier());

		TrainTestFeaturesTools ttft(features, labels,classi);
				
		cout << ttft.crossValidateAll(divisions) << endl;
			
	} else { //separated files
		string file(argv[1]);
		string fileTest(argv[2]);

		Mat features;
		Mat labels;

		Mat testFeatures;
		Mat testLabels;

		MatrixTools::readBin(file, features, labels);
		MatrixTools::readBin(fileTest, testFeatures, testLabels);

		vector<IClassifier*> classi;
		//classi.push_back(new SRClassifier());
		classi.push_back(new kNNClassifier());
		//classi.push_back(new SVMClassifier());
		classi.push_back(new VWBasicClassifier());

		TrainTestFeaturesTools ttft(features, labels, testFeatures, testLabels,classi);
				
		cout << ttft.testAll() << endl;
	
	}
	

	return 0;
}

void testLoadSaveIClassifier(int argc, char *argv[]){

	string file(argv[1]);

	Mat features;
	Mat labels;
	
	MatrixTools::readBin(file, features, labels);

	IClassifier* vw = new SVMClassifier();

	vw->train(features,labels.col(0));
	vw->save("medicalImage_CEDD_SVM");
	
	cout << vw->classify(features.row(0)) << endl;

	delete vw;
	vw = new SVMClassifier();

	vw->load("medicalImage_CEDD_SVM");

	cout << vw->classify(features.row(0)) << endl;
}

void testLoadSaveIIndexer(int argc, char *argv[]){
	string file(argv[1]);

	Mat features;
	Mat labels;
	
	MatrixTools::readBin(file, features, labels);
	IIndexer* vw = new FlannkNNIndexer();

	vw->index(features);
	vw->save("medicalImage_CEDD_kNN");
	Mat q = features.row(0);
	
	vector<std::pair<float,float> > r = vw->knnSearchId(q,10);
	for(uint i = 0; i < r.size(); i++){
		cout << r.at(i).first << "\t" << r.at(i).second << endl;
	}
	cout  << endl;
	delete vw;
	vw = new FlannkNNIndexer();

	vw->load("medicalImage_CEDD_kNN");

	r = vw->knnSearchId(q,10);

	for(uint i = 0; i < r.size(); i++){
		cout << r.at(i).first << "\t" << r.at(i).second << endl;
	}
}

int faceDetectionParameterChallenge(int argc, char *argv[]){


	TextFileSourceV2 is(argv[1]);

	FaceDetection fd(FACE_DETECTION_CASCADE_PATH + "haarcascade_frontalface_alt_tree.xml",FACE_DETECTION_CASCADE_PATH + "haarcascade_eye_tree_eyeglasses.xml",0.5,1, cv::Size(20,20),cv::Size(1000,1000),true);

	Mat src;

	int u = 0;
	int step = 100;
	int size = is.getImageCount();
	double lastT = cvGetTickCount();
	double startT = cvGetTickCount();

	int facesDetected = 0;


	int angles[] = {-20,20};
	int anglesLen = 2;
	while (!(src = is.nextImage()).empty()) {

		vector<Mat> faceImages;
		vector<cv::Point> locations;
		vector<Rect> faceRois;
		fd.detectFaces(src, faceImages, locations, faceRois);

		/*
		cv::Point center;
		center.x = cvRound((src.cols*0.5));
		center.y = cvRound((src.rows*0.5));

		for (int i = 0; i < anglesLen; i++){ 

			vector<Mat> faceImages2;
			vector<cv::Point> locations2;
			vector<Rect> faceRois2;

			double angle = angles[i];

			Mat rotatedImage;

			Mat rot_mat = getRotationMatrix2D(center, angle, 1.0);
			warpAffine(src, rotatedImage, rot_mat, src.size());

			fd.detectFaces(rotatedImage, faceImages, locations, faceRois);
		}*/
		Mat newSrc;

		src.copyTo(newSrc);
		facesDetected += faceRois.size();
		for (uint i = 0; i < faceRois.size(); i++){
			stringstream ss;
			ss << is.getCurrentImageInfoField(0) << "_face_" << i << ".jpg";
			imwrite(ss.str(), faceImages.at(i));
			cv::Rect roi = faceRois.at(i);
			rectangle(newSrc, roi, Scalar(255,255,0));
		
		}

		stringstream ss;

		ss << "faces_" <<	 is.getCurrentImageInfoField(0);
		imwrite(ss.str(), newSrc);
	
		u++;

		//	double rem =  ((double) cvGetTickCount() - lastT)
		//			/ ((double) cvGetTickFrequency() * 1000000);
		//	cout << rem << " seconds elapsed" << endl;
		//	lastT = cvGetTickCount();

		if (u % step == 0) {
			cout << u << endl;
			double rem = ((size - u) / (60 * step))
					* ((double) cvGetTickCount() - lastT)
					/ ((double) cvGetTickFrequency() * 1000000);
			cout << rem << " minutes remaining" << endl;
			lastT = cvGetTickCount();
		}
	}

	cout << "imageCount " << size << endl; 
	cout << "facesDetected " << facesDetected << endl; 
	lastT = cvGetTickCount();
	double rem = (double)(lastT - startT) / ((double) cvGetTickFrequency() * 1000000);
	cout << rem << " seconds elapsed" << endl;
}


void testMSIDXIndexer(int argc, char *argv[]){
	string file(argv[1]);


	int w = atoi(argv[2]);
	int k = atoi(argv[3]);

	Mat features;
	Mat labels;
	
	MatrixTools::readBin(file, features, labels);

	string dummy = "";
	map<string,string> params;
	params["algorithm"] = "linear";
	params["distance"] = "EUCLIDEAN";
	IIndexer* linear = new FlannkNNIndexer(dummy,params);

	map<string,string> paramsL;
	paramsL["algorithm"] = "lsh";
	paramsL["distance"] = "EUCLIDEAN";
	IIndexer* lsh = new FlannkNNIndexer(dummy,params);
	//IIndexer* ms = new FlannkNNIndexer(dummy,params);
	IIndexer* ms = new MSIDXIndexer(dummy,w);

	linear->index(features);
	ms->index(features);

	Mat q = features.row(0);
	
	vector<std::pair<float,float> > r = linear->knnSearchId(q,k);
	for(uint i = 0; i < r.size(); i++){
		cout << r.at(i).first << "\t" << r.at(i).second << endl;
	}
	cout  << endl;
	cout  << endl;
	
	r = ms->knnSearchId(q,k);
	for(uint i = 0; i < r.size(); i++){
		cout << r.at(i).first << "\t" << r.at(i).second << endl;
	}
	cout  << endl;
}

int main(int argc, char *argv[])
{	
	//testLoadSaveIClassifier(argc, argv);
	//testLoadSaveIIndexer(argc, argv);
	//faceDetectionParameterChallenge(argc, argv);
    //testAllClassifiersBin(argc, argv);
	testMSIDXIndexer(argc, argv);
    return 0;
}