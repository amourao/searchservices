#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <time.h>
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
#include "nVector/LBPExtractor.h"
#include "nVector/FeatureExtractor.h"

#include "nVector/NullExtractor.h"
#include "nKeypoint/SURFExtractor.h"
#include "nVector/HistogramExtractor.h"
#include "nVector/SegmentedHistogramExtractor.h"
#include "nVector/LireExtractor.h"


#include "nRoi/FaceDetection.h"

#include "nTag/kNNClassifier.h"
#include "nTag/SVMClassifier.h"
#include "nTag/VWBasicClassifier.h"

#include "tools/TrainTestFeaturesTools.h"
#include "tools/MIRFlickrImporter.h"
#include "tools/tinyImageImporter.h"

#include "FactoryAnalyser.h"

#include "../indexer/FactoryIndexer.h"
#include "../indexer/IIndexer.h"
#include "../indexer/FlannkNNIndexer.h"
#include "../indexer/MSIDXIndexer.h"

#include "../commons/StringTools.h"

#include "../dataModel/DatabaseConnection.h"

#include "../commons/LoadConfig.h"

#include "../rest/Endpoints/analyser/ExtractFeatures.h"
#include "../rest/RestServer.h"


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


void extractAllFeaturesCK(int argc, char *argv[]) {

	string testPath = string(argv[1]);
	TextFileSourceV2 is(testPath);

	//HistogramExtractor histogramExtractor (8);

	vector<cv::Rect> rectangleRois = vector<cv::Rect>();

	 rectangleRois.push_back(cv::Rect(0,0,46,64));
	 rectangleRois.push_back(cv::Rect(46,64,46,112-64));
	 rectangleRois.push_back(cv::Rect(46,0,46,64));
	 rectangleRois.push_back(cv::Rect(0,64,46,112-64));
	 rectangleRois.push_back(cv::Rect(0,10,92,30));
	 rectangleRois.push_back(cv::Rect(20,65,52,30));

	GaborExtractor faceGaborExtractor(92, 112, 4, 6, rectangleRois);
	LBPExtractor lbpExtractor (59, 5, 6, false);



	cv::Mat src;
	cv::Mat gaborNeutral;
	cv::Mat lbpNeutral;

	cv::Mat labelsAll;

	cv::Mat gaborV0ALL;
	cv::Mat gaborV1ALL;
	cv::Mat gaborV2ALL;
	cv::Mat gaborV3ALL;

	cv::Mat lbpV0ALL;
	cv::Mat lbpV1ALL;
	cv::Mat lbpV2ALL;
	cv::Mat lbpV3ALL;

	map<string,cv::Mat> gaborOwnNeutralD;
	map<string,cv::Mat> lbpOwnNeutralD;


	cv::Mat globalNeutral = imread("/home/amourao/data/facialExpressionFeatures/Images/avgFaces/neutral.png");

	cv::Mat globalNeutralGabor;
	faceGaborExtractor.extractFeatures(globalNeutral, globalNeutralGabor);

	cv::Mat globalNeutralLbp;
	lbpExtractor.extractFeatures(globalNeutral, globalNeutralLbp);

	int u = 0;
	int step = 20;
	double lastT = cvGetTickCount();

	int size = is.getImageCount();
	while (!(src = is.nextImage()).empty()) {

		int idI = atoi(is.getCurrentImageInfoField(1).c_str());
		int idC = atoi(is.getCurrentImageInfoField(2).c_str());


		if (gaborOwnNeutralD.find(is.getCurrentImageInfoField(1)) == gaborOwnNeutralD.end() ) {
			stringstream ss1;

			ss1 << is.getCurrentImageInfoField(6) << is.getCurrentImageInfoField(1) << "/neutral.png";

  			cv::Mat ownNeutralIma = imread(ss1.str());
			cv::Mat gaborOwnNeutralTmp;
			cv::Mat lbpOwnNeutralTmp;

			faceGaborExtractor.extractFeatures(ownNeutralIma, gaborOwnNeutralTmp);
			lbpExtractor.extractFeatures(ownNeutralIma, lbpOwnNeutralTmp);

			gaborOwnNeutralD[is.getCurrentImageInfoField(1)] = gaborOwnNeutralTmp;
			lbpOwnNeutralD[is.getCurrentImageInfoField(1)] = lbpOwnNeutralTmp;
		}

		cv::Mat gaborOwnNeutral = gaborOwnNeutralD[is.getCurrentImageInfoField(1)];
		cv::Mat lbpOwnNeutral = lbpOwnNeutralD[is.getCurrentImageInfoField(1)];

		gaborOwnNeutralD[is.getCurrentImageInfoField(1)];
		//float id;
		//float detected;
		//float expected;

		stringstream neutralPath;

		neutralPath << is.getBasePath() << is.getCurrentImageInfoField(5);

		cv::Mat neutral = imread(neutralPath.str());

		cv::Mat gaborV0;
		cv::Mat lbpV0;

		faceGaborExtractor.extractFeatures(src, gaborV0);
		faceGaborExtractor.extractFeatures(neutral, gaborNeutral);

		cv::Mat gaborV1 = gaborV0 - gaborNeutral;
		cv::Mat gaborV2 = gaborV0 - gaborOwnNeutral;
		cv::Mat gaborV3 = gaborV0 - globalNeutralGabor;


		normalize(gaborV1, gaborV1, 0, 1, CV_MINMAX);
		normalize(gaborV2, gaborV2, 0, 1, CV_MINMAX);
		normalize(gaborV3, gaborV3, 0, 1, CV_MINMAX);


		lbpExtractor.extractFeatures(src, lbpV0);
		lbpExtractor.extractFeatures(neutral, lbpNeutral);


		cv::Mat lbpV1 = lbpV0 - lbpNeutral;
		cv::Mat lbpV2 = lbpV0 - lbpOwnNeutral;
		cv::Mat lbpV3 = lbpV0 - globalNeutralLbp;

		normalize(lbpV1, lbpV1, 0, 1, CV_MINMAX);
		normalize(lbpV2, lbpV2, 0, 1, CV_MINMAX);
		normalize(lbpV3, lbpV3, 0, 1, CV_MINMAX);


		gaborV0ALL.push_back(gaborV0);
		gaborV1ALL.push_back(gaborV1);
		gaborV2ALL.push_back(gaborV2);
		gaborV3ALL.push_back(gaborV3);

		lbpV0ALL.push_back(lbpV0);
		lbpV1ALL.push_back(lbpV1);
		lbpV2ALL.push_back(lbpV2);
		lbpV3ALL.push_back(lbpV3);

		Mat labels(1,2,CV_32F);
		labels.at<float>(0,0) = idI;
		labels.at<float>(0,1) = idC;

		labelsAll.push_back(labels);

		u++;

		if (u % step == 0) {
			cout << u << endl;
			double rem = ((size - u) / (60 * step))
					* ((double) cvGetTickCount() - lastT)
					/ ((double) cvGetTickFrequency() * 1000000);
			cout << rem << " minutes remaining" << endl;
			lastT = cvGetTickCount();
		}
	}

	cout << labelsAll << endl;

	/*
	string out = "gaborV0.bin";
	MatrixTools::writeBinV2(out,gaborV0ALL,labelsAll);
	out = "gaborV1.bin";
	MatrixTools::writeBinV2(out,gaborV1ALL,labelsAll);
	out = "gaborV2.bin";
	MatrixTools::writeBinV2(out,gaborV2ALL,labelsAll);
	out = "gaborV3.bin";
	MatrixTools::writeBinV2(out,gaborV3ALL,labelsAll);

	out = "lbpV0.bin";
	MatrixTools::writeBinV2(out,lbpV0ALL,labelsAll);
	out = "lbpV1.bin";
	MatrixTools::writeBinV2(out,lbpV1ALL,labelsAll);
	out = "lbpV2.bin";
	MatrixTools::writeBinV2(out,lbpV2ALL,labelsAll);
	out = "lbpV3.bin";
	MatrixTools::writeBinV2(out,lbpV3ALL,labelsAll);
	*/
}


int testAllClassifiersBin(int argc, char *argv[]) {


	if(argc < 3){
		cout << "usage: analyser <trainBinFile> <testBinFile> ";
		cout << "OR analyser <allDataBinFile> <nrOfDivisions>"  << endl;
		cout << "nrOfDivisions == 9 leads to crossvalidation: 9 parts for train, 1 for test"  << endl;
		exit(1);
	}
	string name(argv[1]);

	int divisions =  atoi(argv[2]);

	vector<IClassifier*> classi; //choose the classifiers to test
	string dummy = "";
	srand (time(NULL));

	unsigned pos = name.rfind("/");
	unsigned pos2 = name.rfind(".");

  	string randf = name.substr(pos+1,pos2);

	stringstream sss;
	sss << "./tmpData/" << randf << "";
	randf = sss.str();
	cout << randf << endl;
	//classi.push_back(new SRClassifier(dummy));
	//classi.push_back(new SVMClassifier(dummy));
	//classi.push_back(new kNNClassifier());
    map<string,string> params;
    params["trainFile"] = randf;

	classi.push_back(new VWBasicClassifier(dummy,params));

	if (isdigit(argv[2][0])){ //crossvalidation

		string file(argv[1]);

		Mat features;
		Mat labels;

		MatrixTools::readBin(file, features, labels);

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

	std::pair<vector<float>,vector<float> > r = vw->knnSearchId(q,10);
	for(uint i = 0; i < r.first.size(); i++){
		cout << r.first.at(i) << "\t" << r.second.at(i) << endl;
	}
	cout  << endl;
	delete vw;
	vw = new FlannkNNIndexer();

	vw->load("medicalImage_CEDD_kNN");

	r = vw->knnSearchId(q,10);

	for(uint i = 0; i < r.first.size(); i++){
		cout << r.first.at(i) << "\t" << r.second.at(i) << endl;
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


	//int angles[] = {-20,20};
	//int anglesLen = 2;
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

	return 0;
}


void testMSIDXIndexer(int argc, char *argv[]){
	string file(argv[1]);


	int n = atoi(argv[2]);
	//int w = atoi(argv[3]);
	int k = atoi(argv[4]);

	Mat features;
	//Mat labels;

	tinyImageImporter ti;
	ti.readBin(file,n,features);
	//MatrixTools::readBin(file, features, labels);
	string dummy = "";
	map<string,string> params;
	params["algorithm"] = "linear";
	params["distance"] = "EUCLIDEAN";
	IIndexer* linear = new FlannkNNIndexer(dummy,params);

	map<string,string> paramsL;
	paramsL["algorithm"] = "lsh";
	paramsL["distance"] = "EUCLIDEAN";
	paramsL["table_number"] = "5";
	paramsL["key_size"] = "10";
	paramsL["multi_probe_level"] = "0";
	//table_number the number of hash tables to use [10...30]
    //key_size the size of the hash key in bits [10...20]
    //multi_probe_level the number of bits to shift to check for neighboring buckets
    //(0 is regular LSH, 2 is recommended).
	IIndexer* lsh = new FlannkNNIndexer(dummy,params);

    map<string,string> paramsMSIDX;
	paramsMSIDX["w"] = string(argv[3]);
	IIndexer* ms = new MSIDXIndexer(dummy,paramsMSIDX);

	linear->index(features);
	lsh->index(features);
	ms->index(features);

	lsh->save("L");
	lsh->load("L");
	Mat q = features.row(0);

	std::pair< vector<float> , vector<float> > r = linear->knnSearchId(q,k);
	for(uint i = 0; i < r.first.size(); i++){
		cout << r.first.at(i)<< "\t" << r.second.at(i) << endl;
	}

	cout  << endl;
	cout  << endl;

	r = lsh->knnSearchId(q,k);
	for(uint i = 0; i < r.first.size(); i++){
		cout << r.first.at(i)<< "\t" << r.second.at(i) << endl;
	}

	cout  << endl;
	cout  << endl;

	r = ms->knnSearchId(q,k);
	for(uint i = 0; i < r.first.size(); i++){
		cout << r.first.at(i)<< "\t" << r.second.at(i) << endl;
	}
}

int merger(int argc, char *argv[]){
	string a = argv[1];
	stringstream ss;
	ss << a << "_";
	string b = ss.str();

	stringstream ss1;
	ss1 << "../" << a;
	string c = ss1.str();

	Mat t1;
	Mat t2;

	Mat l1;
	Mat l2;

	MatrixTools::readBin(a,t1,l1);
	MatrixTools::readBin(b,t2,l2);

	t1.push_back(t2);
	l1.push_back(l2);

	MatrixTools::writeBinV2(c,t1,l1);

	return 0;
}

//normalize(dst, dst, 0,255, CV_MINMAX);

void createMedCatClassifier(int argc, char *argv[]) {

    string trainData = string(argv[1]);
	TextFileSourceV2 is(trainData);

	Mat src;

    string aa = "cedd";
    map<string,string> params;
    params["algorithm"] = "cedd";
    LireExtractor ceddExtractor (aa,params);

    aa = "fcth";
    map<string,string> paramsB;
    paramsB["algorithm"] = "fcth";
    LireExtractor fcthExtractor (aa,paramsB);

    int i = -1;
    int j = 0;
    string lastCateg = "";

    Mat features, labels;

    while (j < is.getImageCount()) {
        src = is.nextImage();
        if (!src.empty()){
        string currentCat = is.getCurrentImageInfoField(2);

        if (currentCat != lastCateg){
            i++;
            lastCateg = currentCat;
        }


        if (j % 100 == 0)
            cout << j << " " << is.getImageCount() << endl;

        Mat f1, f2, comb;
        Mat label(1,1,CV_32F);

        ceddExtractor.extractFeatures(src, f1);
        fcthExtractor.extractFeatures(src, f2);

        hconcat(f1,f2,comb);

        label.at<float>(0,0) = i;
        features.push_back(comb);
        labels.push_back(label);
        }
        j++;
    }

    cout << "Wrote files" << endl;

    string type = "medClassifier";

    MatrixTools::writeBinV2(type,features,labels);

    SVMClassifier svm(type);

    cout << "Training" << endl;
    svm.train(features,labels);

    cout << "Saving" << endl;
    svm.save(type);


}

void extractAllFeaturesCKv2(int argc, char *argv[]) {

	string testPath = string(argv[1]);
	TextFileSourceV2 is(testPath);

	//HistogramExtractor histogramExtractor (8);

	vector<cv::Rect> rectangleRois = vector<cv::Rect>();

	 rectangleRois.push_back(cv::Rect(0,0,46,64));
	 rectangleRois.push_back(cv::Rect(46,64,46,112-64));
	 rectangleRois.push_back(cv::Rect(46,0,46,64));
	 rectangleRois.push_back(cv::Rect(0,64,46,112-64));
	 rectangleRois.push_back(cv::Rect(0,10,92,30));
	 rectangleRois.push_back(cv::Rect(20,65,52,30));

	GaborExtractor faceGaborExtractor(92, 112, 4, 6, rectangleRois);
	//LBPExtractor lbpExtractor (59, 5, 6, false);

	//LBPExtractor faceGaborExtractor (59, 5, 6, false);



	cv::Mat src;
	cv::Mat gaborNeutral;
	cv::Mat lbpNeutral;

	cv::Mat labelsAll;

	cv::Mat gaborV0ALL;
	cv::Mat gaborV1ALL;
	cv::Mat gaborV2ALL;
	cv::Mat gaborV3ALL;

	map<string,cv::Mat> gaborOwnNeutralD;


	cv::Mat globalNeutral = imread("/home/amourao/data/facialExpressionFeatures/Images/avgFaces/neutral.png");

	cv::Mat globalNeutralGabor;
	faceGaborExtractor.extractFeatures(globalNeutral, globalNeutralGabor);
	normalize(globalNeutralGabor, globalNeutralGabor, 0,1, CV_MINMAX);

	int u = 0;
	int step = 20;
	double lastT = cvGetTickCount();

	int size = is.getImageCount();
	while (!(src = is.nextImage()).empty()) {

		int idI = atoi(is.getCurrentImageInfoField(1).c_str());
		int idC = atoi(is.getCurrentImageInfoField(2).c_str());


		if (gaborOwnNeutralD.find(is.getCurrentImageInfoField(1)) == gaborOwnNeutralD.end() ) {
			stringstream ss1;

			ss1 << is.getCurrentImageInfoField(6) << is.getCurrentImageInfoField(1) << "/neutral.png";

  			cv::Mat ownNeutralIma = imread(ss1.str());
			cv::Mat gaborOwnNeutralTmp;

			faceGaborExtractor.extractFeatures(ownNeutralIma, gaborOwnNeutralTmp);

			normalize(gaborOwnNeutralTmp, gaborOwnNeutralTmp, 0,1, CV_MINMAX);


			gaborOwnNeutralD[is.getCurrentImageInfoField(1)] = gaborOwnNeutralTmp;
		}

		//int isd = 0;
		//int isd2 = 50;

		cv::Mat gaborOwnNeutral = gaborOwnNeutralD[is.getCurrentImageInfoField(1)];

		gaborOwnNeutralD[is.getCurrentImageInfoField(1)];
		//float id;
		//float detected;
		//float expected;

		stringstream neutralPath;

		neutralPath << is.getBasePath() << is.getCurrentImageInfoField(5);

		cv::Mat neutral = imread(neutralPath.str());

		cv::Mat gaborV0;
		cv::Mat lbpV0;

		faceGaborExtractor.extractFeatures(src, gaborV0);
		faceGaborExtractor.extractFeatures(neutral, gaborNeutral);

		normalize(gaborV0, gaborV0, 0,1, CV_MINMAX);
		normalize(gaborNeutral, gaborNeutral, 0,1, CV_MINMAX);

		cv::Mat gaborV1 = gaborV0 - gaborNeutral;
		cv::Mat gaborV2 = gaborV0 - gaborOwnNeutral;
		cv::Mat gaborV3 = gaborV0 - globalNeutralGabor;


		normalize(gaborV1, gaborV1, 0, 1, CV_MINMAX);
		normalize(gaborV2, gaborV2, 0, 1, CV_MINMAX);
		normalize(gaborV3, gaborV3, 0, 1, CV_MINMAX);

		gaborV0ALL.push_back(gaborV0);
		gaborV1ALL.push_back(gaborV1);
		gaborV2ALL.push_back(gaborV2);
		gaborV3ALL.push_back(gaborV3);

		Mat labels(1,2,CV_32F);
		labels.at<float>(0,0) = idI;
		labels.at<float>(0,1) = idC;

		labelsAll.push_back(labels);

		u++;

		if (u % step == 0) {
			cout << u << endl;
			double rem = ((size - u) / (60 * step))
					* ((double) cvGetTickCount() - lastT)
					/ ((double) cvGetTickFrequency() * 1000000);
			cout << rem << " minutes remaining" << endl;
			lastT = cvGetTickCount();
		}
	}

	string ext = faceGaborExtractor.getName();
	string out;
	stringstream ss;
	ss << ext << "V0.bin";
	out = ss.str();
	ss.str(std::string());
	MatrixTools::writeBinV2(out,gaborV0ALL,labelsAll);

	ss << ext << "V1.bin";
	out = ss.str();
	ss.str(std::string());
	MatrixTools::writeBinV2(out,gaborV1ALL,labelsAll);

	ss << ext << "V2.bin";
	out = ss.str();
	ss.str(std::string());
	MatrixTools::writeBinV2(out,gaborV2ALL,labelsAll);

	ss << ext << "V3.bin";
	out = ss.str();
	ss.str(std::string());
	MatrixTools::writeBinV2(out,gaborV3ALL,labelsAll);
}

void classifyAllImages(int argc, char *argv[]) {

    string type = "medClassifier";

    SVMClassifier svm(type);

    cout << "Loading" << endl;
    svm.load(type);

    string trainData = string(argv[1]);
	TextFileSourceV2 is(trainData);

	Mat src;

    string aa = "cedd";
    map<string,string> params;
    params["algorithm"] = "cedd";
    LireExtractor ceddExtractor (aa,params);

    aa = "fcth";
    map<string,string> paramsB;
    paramsB["algorithm"] = "fcth";
    LireExtractor fcthExtractor (aa,paramsB);

    //int i = -1;
    int j = 0;
    string lastCateg = "";

    Mat features, labels;

    while (j < is.getImageCount()) {
        src = is.nextImage();
        //cout << is.getImagePath() << endl;

        //cout <<  StringTools::split(is.getCurrentImageInfoField(0),'.')[0] << endl;
        if (!src.empty()){
        string iri = StringTools::split(is.getCurrentImageInfoField(0),'.')[0];


        if (j % 100 == 0)
            cout << j << " " << is.getImageCount() << endl;

        Mat f1, f2, comb;
        Mat label(1,1,CV_32F);

        ceddExtractor.extractFeatures(src, f1);
        fcthExtractor.extractFeatures(src, f2);

        hconcat(f1,f2,comb);

        float f = svm.classify(comb);

        label.at<float>(0,0) = f;
        features.push_back(comb);
        labels.push_back(label);

        cout << is.getCurrentImageInfoField(0) << ";" << iri << ";" << f << ";" << j << endl;
        }
        j++;
    }

    type = "medFeatures";
    MatrixTools::writeBinV2(type,features,labels);
    cout << "Wrote files" << endl;

}


void classifyAllImagesCondor(int argc, char *argv[]) {

    string type = "medClassifier";

    SVMClassifier svm(type);

    cout << "Loading" << endl;
    svm.load(type);

    string trainData = string(argv[1]);
    int myDivision = atoi(argv[2]);
    int totalDivisions = atoi(argv[3]);

    TextFileSourceV2 is(trainData);

    int imageCount = is.getImageCount();

    int imagesToProcess = (imageCount/totalDivisions)+1;
    int startAt = imagesToProcess*myDivision;

    if ((imagesToProcess+startAt)>imageCount){
        imagesToProcess = imageCount-startAt;
    }

    is.skipTo(startAt);

	Mat src;

    string aa = "cedd";
    map<string,string> params;
    params["algorithm"] = "cedd";
    LireExtractor ceddExtractor (aa,params);

    aa = "fcth";
    map<string,string> paramsB;
    paramsB["algorithm"] = "fcth";
    LireExtractor fcthExtractor (aa,paramsB);

    //int i = -1;
    //int j = 0;
    string lastCateg = "";

    Mat features, labels;

    for (int j = 0; j < imagesToProcess; j++) {
        try{
        src = is.nextImage();
        //cout << is.getImagePath() << endl;

        //cout <<  StringTools::split(is.getCurrentImageInfoField(0),'.')[0] << endl;
        if (!src.empty()){
        string iri = StringTools::split(is.getCurrentImageInfoField(0),'.')[0];

        Mat f1, f2, comb;
        Mat label(1,1,CV_32F);

        ceddExtractor.extractFeatures(src, f1);
        fcthExtractor.extractFeatures(src, f2);

        hconcat(f1,f2,comb);

        float f = svm.classify(comb);

        label.at<float>(0,0) = f;
        features.push_back(comb);
        labels.push_back(label);

        cout << is.getCurrentImageInfoField(0) << ";" << iri << ";" << f << ";" << j << ";" << j+startAt << endl;
        }
        } catch(...){

        }
    }

    stringstream ss;
    ss << "medFeatures_";
    ss << std::setw(2) << std::setfill('0') << myDivision;
    string filename = ss.str();
    MatrixTools::writeBinV2(filename,features,labels);
    cout << "Wrote files" << endl;

}


void classifyAllBlipImagesCondor(int argc, char *argv[]) {

    string paramFile(argv[1]);
    int myDivision = atoi(argv[2]);
    int divisionOffset = atoi(argv[3]);
    int totalDivisions = atoi(argv[4]);


    myDivision += divisionOffset;

	map<string,string> parameters;

	LoadConfig::load(paramFile,parameters);

    TextFileSourceV2 is(parameters["infilename"]);

    ofstream fout;
    stringstream ss;
    ss << parameters["outfilename"] << "_";
    ss << std::setw(2) << std::setfill('0') << myDivision << ".txt";
    fout.open(ss.str().c_str());

    ofstream foutRoi;
    stringstream ss2;
    ss2 << parameters["outfilenameROI"] << "_";
    ss2 << std::setw(2) << std::setfill('0') << myDivision << ".txt";
    foutRoi.open(ss2.str().c_str());


    int imageCount = is.getImageCount();

    int imagesToProcess = (imageCount/totalDivisions)+1;
    int startAt = imagesToProcess*myDivision;

    if ((imagesToProcess+startAt)>imageCount){
        imagesToProcess = imageCount-startAt;
    }

    is.skipTo(startAt);

	Mat src;


	Mat vocabulary, labelsAll;
	string outV = "/localstore/amourao/code/vocabulary.bin";
	MatrixTools::readBinV2(outV,vocabulary,labelsAll);
	BOWImgDescriptorExtractor bowMatcher(DescriptorExtractor::create("SIFT"),DescriptorMatcher::create("BruteForce"));
	bowMatcher.setVocabulary(vocabulary);


    vector<FeatureExtractor*> fExtractors;
    vector<KeypointFeatureExtractor*> kExtractors;
    vector<RoiFeatureExtractor*> rExtractors;

    vector<string> fExtractorsS = StringTools::split(parameters["vectorAnalysers"],',');

    for(uint i = 0; i < fExtractorsS.size(); i++)
        fExtractors.push_back((FeatureExtractor*)FactoryAnalyser::getInstance()->createType(fExtractorsS.at(i)));

    vector<string> kExtractorsS = StringTools::split(parameters["pointAnalysers"],',');

    for(uint i = 0; i < kExtractorsS.size(); i++)
        kExtractors.push_back((KeypointFeatureExtractor*)FactoryAnalyser::getInstance()->createType(kExtractorsS.at(i)));

    vector<string> rExtractorsS = StringTools::split(parameters["roiAnalysers"],',');

    for(uint i = 0; i < rExtractorsS.size(); i++)
        rExtractors.push_back((RoiFeatureExtractor*)FactoryAnalyser::getInstance()->createType(rExtractorsS.at(i)));

    //int i = -1;
    int totalImages = 0;
    string lastCateg = "";

    for (int j = 0; j < imagesToProcess; j++) {
        try{
            src = is.nextImage();

            //cout <<  StringTools::split(is.getCurrentImageInfoField(0),'.')[0] << endl;
            if (!src.empty()){

            	//foutRoi << endl;
            	//foutRoi << is.getImageInfo() << ";" << totalImages << ";" << totalImages+startAt  <<endl;
                fout << is.getImageInfo() << ";" << totalImages << ";" << totalImages+startAt << endl;



            	Mat labels(1,2,CV_32F);

                labels.at<float>(0,0) = totalImages + startAt;
                labels.at<float>(0,1) = totalImages;


                totalImages++;
                for(uint i = 0; i < fExtractors.size(); i++){
                    Mat features;
                    fExtractors.at(i)->extractFeatures(src,features);
                	stringstream ss;
			        ss << "blip_" << fExtractors.at(i)->getName() << "_";
			        ss << std::setw(2) << std::setfill('0') << myDivision << ".bin";
			        string filename = ss.str();
			        MatrixTools::writeBinV2(filename,features,labels,true);
			        features.release();
                }


                for(uint i = 0; i < kExtractors.size(); i++){
                    Mat features;
                    vector<KeyPoint> keypoints;
                    kExtractors.at(i)->extractFeatures(src,keypoints,features);

                    stringstream ss;
        			ss << "blip_" << kExtractors.at(i)->getName() << "_";
        			ss << std::setw(2) << std::setfill('0') << myDivision << ".bin";
        			string filename = ss.str();
        			//MatrixTools::writeBinV3(filename,features,keypoints,labels,true);
        			features.release();
        			//if (kExtractors.at(i)->getName() == "SIFTExtractor"){
        				Mat imgDescriptor;
        				bowMatcher.compute(src, keypoints, imgDescriptor);
        				stringstream ss2;
        				ss2 << "blip_" << kExtractors.at(i)->getName() << "_bow_";
        				ss2 << std::setw(2) << std::setfill('0') << myDivision << ".bin";
        				string filename2 = ss2.str();
        				//MatrixTools::writeBinV2(filename2,imgDescriptor,labels,true);
        				foutRoi << totalImages+startAt << ";" << is.getImageInfo() <<endl;
        			//}
                }

                for(uint i = 0; i < rExtractors.size(); i++){
                    map<string,region> features;
                    rExtractors.at(i)->extractFeatures(src,features);
                    map<string,region>::iterator iter;
                    for (iter = features.begin(); iter != features.end(); ++iter){
                        //foutRoi << totalImages-1 << ";" << iter->second.annotationType << ";" << iter->second.x << ";" << iter->second.y << ";";
                        //foutRoi << iter->second.width << ";" << iter->second.height << endl;
                    }
                }

                labels.release();
                src.release();

            } else  {
               cout << "A: " << is.getImageInfo() << endl;
            }
        } catch(const std::exception &e){
            cout << "B: " << is.getImageInfo() << endl;
            cout << e.what() << endl;
            cout << "Missed B" << endl;
        } catch(...){
            cout << "C: " << is.getImageInfo() << endl;
            //cout << "Missed C" << endl;
        }
    }
}


int testDatabaseConnection(int argc, char *argv[]){
    DatabaseConnection db;

    vector<string> keys;
    vector<string> values;

    keys.push_back("iri");
    values.push_back("1471-2091-1-1-5");

    vector<map<string,string> > s = db.getRows("images","doi,iri,caption",keys,values,true,-1);

    return 0;

}

int testBinFormat(int argc, char *argv[]){

    string paramFile(argv[1]);

	map<string,string> parameters;

	LoadConfig::load(paramFile,parameters);

    FeatureDetector* detector = new SiftFeatureDetector(0, // nFeatures
                                                        4, // nOctaveLayers
                                                        0.04, // contrastThreshold
                                                        10, //edgeThreshold
                                                        1.6 //sigma
                                                        );
    DescriptorExtractor* extractor = new SiftDescriptorExtractor();

    vector<KeyPoint> keypoints;
    Mat descriptors;

    Mat originalGrayImage = imread(parameters["filename"]);

    detector->detect(originalGrayImage, keypoints);
    extractor->compute(originalGrayImage, keypoints, descriptors);

    Mat label(1,2,CV_32F);

    label.at<float>(0,0) = descriptors.cols;
    label.at<float>(0,1) = descriptors.cols;

    vector<Mat> descriptorsMat;
    descriptorsMat.push_back(descriptors);

    vector<vector<KeyPoint> > keypointsVec;
    keypointsVec.push_back(keypoints);

    string s = parameters["outfilename"];

    MatrixTools::writeBinV2(s,descriptorsMat,label,true);
    //MatrixTools::writeBinV3(s,descriptorsMat,keypointsVec,label,true);

    cout << descriptorsMat.at(0).row(0).colRange(0,2) << " " << descriptorsMat.at(0).rows << " " << label.cols << " " << label.rows << " " << keypointsVec.at(0).at(0).pt.x << " " << keypointsVec.at(0).at(1).pt.y << endl;

    vector<Mat> descriptorsMat2;
    Mat label2;
    MatrixTools::readBinV2(s,descriptorsMat2,label2);

    cout << descriptorsMat2.at(0).row(0).colRange(0,2) << " " << descriptorsMat2.at(0).rows << " " << label2.cols << " " << label2.rows << endl;

    for (uint i = 0; i < descriptorsMat2.size(); i++)
        cout << cv::countNonZero(descriptorsMat.at(i) != descriptorsMat2.at(i)) << endl;


    for (uint i = 0; i < keypoints.size(); i++)
        circle(originalGrayImage,keypoints.at(i).pt,1,CV_RGB(0,255,0));
    imshow("a",originalGrayImage);
    waitKey();

    return 0;
}

int createBlipKnnVWDict(int argc, char *argv[]){

	/*
	string paramFile(argv[1]);

	vector<cv::Mat> features;
	vector<vector<cv::KeyPoint> > keypoints;
	cv::Mat labels;

	cout << "reading" << endl;
	MatrixTools::readBinV3(paramFile,features,keypoints,labels);

	Mat bigFeatures;

	int maxSize = 500000;
	int K = 1000;

	cout << "making big mat" << endl;
	for(uint i = 0; i < features.size() && bigFeatures.rows < maxSize; i++)
		bigFeatures.push_back(features.at(i));
	cout << "bigFeatures: "<< bigFeatures.rows << " " << bigFeatures.cols << endl;
	Mat bestLabels, clustered;
	Mat centers(K, bigFeatures.cols, CV_32F);

	cout << "Training kmeans" << endl;
	BOWKMeansTrainer bowTrainer (K, TermCriteria( CV_TERMCRIT_ITER, 50, 1),5, KMEANS_PP_CENTERS);
	bowTrainer.add(bigFeatures);
	cout << "running kmeans" << endl;
	Mat vocabulary = bowTrainer.cluster();*/
	cout << "read vocabulary" << endl;

	Mat vocabulary, labelsAll;
	string outV = "/localstore/amourao/blip/vocabulary.bin";
	MatrixTools::readBinV2(outV,vocabulary,labelsAll);

	cout << "setting vocabulary" << endl;
	Ptr<FeatureDetector> detector = new SiftFeatureDetector(0, // nFeatures
                                                        4, // nOctaveLayers
                                                        0.04, // contrastThreshold
                                                        10, //edgeThreshold
                                                        1.6 //sigma
                                                        );
	BOWImgDescriptorExtractor bowMatcher(DescriptorExtractor::create("SIFT"),DescriptorMatcher::create("BruteForce"));
	bowMatcher.setVocabulary(vocabulary);

	Mat imgDescriptor;
	vector<KeyPoint> keypoints2;
	Mat image = imread("/home/amourao/code/searchservices/bush.jpg");
	detector->detect(image, keypoints2);

	cout << "extracting demo image" << endl;
	bowMatcher.compute(image, keypoints2, imgDescriptor);

	Mat tempLabels;


	string outD = "/localstore/amourao/blip/descriptor.bin";

	cout << "writing outfiles" << endl;


	cout << bowMatcher.descriptorSize() << " " << bowMatcher.descriptorType() << endl;
	cout << image.cols << " " << image.rows << endl;
	cout << imgDescriptor.cols << " " << imgDescriptor.rows << endl;

	MatrixTools::writeBinV2(outD,imgDescriptor,tempLabels);

	return 0;

}

int extractREST(int argc, char *argv[]){

    map<string, string> parameters;
    string paramFile(argv[5]);

	LoadConfig::load(paramFile,parameters);

    map<string,string> params;

    params["input"] = string(argv[1]);
	params["analyser"] = string(argv[2]);
	params["task"] = string(argv[3]);
	params["output"] = string(argv[4]);

	ExtractFeatures ef ("a");
    ef.getFeatures(params);


    return 0;
}

int classifySapo(int argc, char *argv[]){
    string paramFile(argv[1]);

    double splitTrainPos = atof(argv[1]);
    double splitTestPos = atof(argv[2]);

    double splitTrainNeg = atof(argv[3]);
    double splitTestNeg = atof(argv[4]);

    string positiveExamples = argv[5];

    string negatives = argv[6];
    vector<string> negativeExamples = StringTools::split(negatives,',');

    Mat featuresNegTrain;
    Mat featuresNegTrainL;
    Mat featuresNegTest;
    Mat featuresNegTestL;

    //get positive examples
    //number of examples is strictly based on the ratios provided as parameters

    Mat featuresPos, labelsPos;
    MatrixTools::readBin(positiveExamples,featuresPos,labelsPos);

    int posCountTrain = featuresPos.rows*splitTrainPos;
    int posCountTest = featuresPos.rows*splitTestPos;

    Mat featurePosTrain = featuresPos.rowRange(0,posCountTrain);
    Mat featuresPosTest = featuresPos.rowRange(posCountTrain,posCountTrain+posCountTest);

    Mat featurePosTrainL = Mat(posCountTrain,1, CV_32F, 1.0);
    Mat featuresPosTestL = Mat(posCountTest,1, CV_32F, 1.0);

    //get negative examples
    //number of examples is counted as a porportion of positive examples
    //divided equaly across negative classes

    int negCountTrainExpected = posCountTrain*splitTrainNeg;
    int negCountTestExpected = posCountTest*splitTestNeg;

    int negCountTrain = 0;
    int negCountTest = 0;

    int negCountTrainExpectedPerClass = negCountTrainExpected/negativeExamples.size();
    int negCountTestExpectedPerClass = negCountTestExpected/negativeExamples.size();

    for(string negExample: negativeExamples){
        Mat featuresS, labelsS;
        MatrixTools::readBin(negExample,featuresS,labelsS);

        int currSplitTrain = negCountTrainExpectedPerClass;
        int currSplitTest = negCountTestExpectedPerClass;

        //if the number of features required is too big, split all data using the same train/test ratio
        if((currSplitTrain+currSplitTest)> featuresS.rows){
            currSplitTrain = featuresS.rows * negCountTrainExpected/(negCountTestExpected+negCountTrainExpected);
            currSplitTest = featuresS.rows - currSplitTrain;
        }

        Mat featuresNegTrainC = featuresS.rowRange(0,currSplitTrain);
        Mat featuresNegTestC = featuresS.rowRange(currSplitTrain,currSplitTrain+currSplitTest);

        Mat featuresNegTrainLC = Mat(currSplitTrain,1, CV_32F, 0.0);
        Mat featuresNegTestLC = Mat(currSplitTest,1, CV_32F, 0.0);

        featuresNegTrain.push_back(featuresNegTrainC);
        featuresNegTrainL.push_back(featuresNegTrainLC);

        featuresNegTest.push_back(featuresNegTestC);
        featuresNegTestL.push_back(featuresNegTestLC);

        negCountTrain+=currSplitTrain;
        negCountTest+=currSplitTest;
    }




    Mat train;
    Mat trainL;
    Mat test;
    Mat testL;

    vconcat(featurePosTrain, featuresNegTrain, train);
    vconcat(featurePosTrainL, featuresNegTrainL, trainL);
    vconcat(featuresPosTest, featuresNegTest, test);
    vconcat(featuresPosTestL, featuresNegTestL, testL);

    string type = "sapo";
    map<string,string> p;
    SVMClassifier svm(type,p);

    cout << "Read features ok" << endl << "Training with " << posCountTrain << " pos and " << negCountTrain << " neg" << endl;
    svm.train(train,trainL);
    cout << "Training ok" << endl << "Testing with " << posCountTest << " pos and " << negCountTest << " neg" << endl;
    int correctPos = 0;
    int wrongPos = 0;

    int correctNeg = 0;
    int wrongNeg = 0;
    for(int i = 0; i < test.rows; i++){
        Mat features = test.row(i);

        float label = testL.at<float>(i,0);

        float detected = svm.classify(features);

        if (label == detected && label == 1)
            correctPos++;
        else if (label == detected && label == 0)
            correctNeg++;
        else if (label != detected && label == 1)
            wrongPos++;
        else if (label != detected && label == 0)
            wrongNeg++;

    }
    cout << "Test ok" << endl;
    int correct = correctPos+correctNeg;
    int wrong = wrongPos+wrongNeg;

    cout << (correct/double(correct+wrong))*100 << " %: tp: " << correctPos << " tn: " << correctNeg << " fp: " << wrongNeg  <<  " fn: " << wrongPos << endl;

    return 0;
}


int classifySapoAllVideos(int argc, char *argv[]){
    string paramFile(argv[1]);

    double splitTrainPos = atof(argv[1]);
    double splitTestPos = atof(argv[2]);

    double splitTrainNeg = atof(argv[3]);
    double splitTestNeg = atof(argv[4]);

    string positiveExamples = argv[5];

    string negatives = argv[6];

    bool onlyMiddleKeyframes = (argc > 7 && string(argv[7]) == "middleOnly");



    vector<string> negativeExamples = StringTools::split(negatives,',');

    Mat featuresNegTrain;
    Mat featuresNegTrainL;
    Mat featuresNegTrainLOriginal;
    Mat featuresNegTest;
    Mat featuresNegTestL;
    Mat featuresNegTestLOriginal;

    //get positive examples
    //number of examples is strictly based on the ratios provided as parameters

    Mat featuresPos, labelsPos;
    MatrixTools::readBin(positiveExamples,featuresPos,labelsPos);

    int posCountTrain = featuresPos.rows*splitTrainPos;
    int posCountTest = featuresPos.rows*splitTestPos;

    int lastVideo = labelsPos.at<float>(posCountTrain,1);

    //go until last frame of the current video to avoid spliting videos across train/test data
    while(labelsPos.at<float>(++posCountTrain,1) == lastVideo)
        ;
    posCountTrain--;

    int lastTestValue = min(posCountTrain+posCountTest,labelsPos.rows);
    lastVideo = labelsPos.at<float>(lastTestValue,1);
    //go until last frame of the current video to avoid spliting videos across train/test data
    while(lastTestValue < labelsPos.rows && labelsPos.at<float>(++lastTestValue,1) == lastVideo)
        ;
    lastTestValue--;
    posCountTest = lastTestValue-posCountTrain;

    Mat featurePosTrain = featuresPos.rowRange(0,posCountTrain);
    Mat featurePosTrainLOriginal = labelsPos.rowRange(0,posCountTrain);
    Mat featuresPosTestLOriginal = labelsPos.rowRange(posCountTrain,lastTestValue);
    Mat featuresPosTest = featuresPos.rowRange(posCountTrain,lastTestValue);

    Mat featurePosTrainL= Mat::ones(posCountTrain,1, CV_32F);
    Mat featuresPosTestL= Mat::ones(featuresPosTest.rows,1, CV_32F);


    //option to select only frames from the middle of the scene
    if(onlyMiddleKeyframes){

        cout << "Using only middle keyframes positive examples" << endl;

        Mat tmpfeaturePosTrain;
        Mat tmpfeaturePosTrainL;
        Mat tmpfeaturePosTrainLOriginal;

        featurePosTrain.copyTo(tmpfeaturePosTrain);
        featurePosTrainL.copyTo(tmpfeaturePosTrainL);
        featurePosTrainLOriginal.copyTo(tmpfeaturePosTrainLOriginal);

        featurePosTrain = Mat();
        featurePosTrainL = Mat();
        featurePosTrainLOriginal = Mat();

        bool isMiddleFrame = false;
        float lastVideoId = -1;
        for(int i = 0; i < tmpfeaturePosTrain.rows; i++){
            isMiddleFrame = !isMiddleFrame; //swap to add every other frame
            float videoId = tmpfeaturePosTrainLOriginal.at<float>(i,1);
            if(lastVideoId != videoId) //except when video changes (last frame of video1 next to first frame from video2
                isMiddleFrame = false;
            if (isMiddleFrame){
                featurePosTrain.push_back(tmpfeaturePosTrain.row(i));
                featurePosTrainL.push_back(tmpfeaturePosTrainL.row(i));
                featurePosTrainLOriginal.push_back(tmpfeaturePosTrainLOriginal.row(i));
            }
            lastVideoId = videoId;
        }

        posCountTrain = featurePosTrain.rows;

    }


    //get negative examples
    //number of examples is counted as a porportion of positive examples
    //divided equaly across negative classes

    int negCountTrainExpected = posCountTrain*splitTrainNeg;
    int negCountTestExpected = posCountTest*splitTestNeg;

    int negCountTrain = 0;
    int negCountTest = 0;

    int negCountTrainExpectedPerClass = negCountTrainExpected/negativeExamples.size();
    int negCountTestExpectedPerClass = negCountTestExpected/negativeExamples.size();

    for(string negExample: negativeExamples){
        Mat featuresS, labelsS;
        MatrixTools::readBin(negExample,featuresS,labelsS);

        int currSplitTrain = negCountTrainExpectedPerClass;
        int currSplitTest = negCountTestExpectedPerClass;

        //if the number of features required is too big, split all data using the same train/test ratio
        if((currSplitTrain+currSplitTest)> featuresS.rows){
            currSplitTrain = featuresS.rows * negCountTrainExpected/(negCountTestExpected+negCountTrainExpected);
            currSplitTest = featuresS.rows - currSplitTrain;
        }

        lastVideo = labelsS.at<float>(currSplitTrain,1);

        //go until last frame of the current video to avoid spliting videos across train/test data
        while(labelsS.at<float>(++currSplitTrain,1) == lastVideo)
            ;
        currSplitTrain--;

        lastTestValue = min(currSplitTrain+currSplitTest,labelsS.rows);
        lastVideo = labelsS.at<float>(lastTestValue,1);

        //go until last frame of the current video to avoid spliting videos across train/test data
        while(lastTestValue++ < labelsS.rows && labelsS.at<float>(lastTestValue,1) == lastVideo)
            ;
        lastTestValue--;

        Mat featuresNegTrainC = featuresS.rowRange(0,currSplitTrain);
        Mat featuresNegTrainLCOriginal = labelsS.rowRange(0,currSplitTrain);
        Mat featuresNegTestC = featuresS.rowRange(currSplitTrain,lastTestValue);
        Mat featuresNegTestLCOriginal = labelsS.rowRange(currSplitTrain,lastTestValue);

        currSplitTest = featuresNegTestC.rows;

        Mat featuresNegTrainLC = Mat::zeros(currSplitTrain,1, CV_32F);
        Mat featuresNegTestLC = Mat::zeros(currSplitTest,1, CV_32F);

        featuresNegTrain.push_back(featuresNegTrainC);
        featuresNegTrainL.push_back(featuresNegTrainLC);
        featuresNegTrainLOriginal.push_back(featuresNegTrainLCOriginal);

        featuresNegTest.push_back(featuresNegTestC);
        featuresNegTestL.push_back(featuresNegTestLC);
        featuresNegTestLOriginal.push_back(featuresNegTestLCOriginal);

        negCountTrain+=currSplitTrain;
        negCountTest+=currSplitTest;
    }




    Mat train;
    Mat trainL;
    Mat trainLOriginal;
    Mat test;
    Mat testL;
    Mat testLOriginal;

    vconcat(featurePosTrain, featuresNegTrain, train);
    vconcat(featurePosTrainL, featuresNegTrainL, trainL);
    vconcat(featurePosTrainLOriginal, featuresNegTrainLOriginal, trainLOriginal);
    vconcat(featuresPosTest, featuresNegTest, test);
    vconcat(featuresPosTestL, featuresNegTestL, testL);
    vconcat(featuresPosTestLOriginal, featuresNegTestLOriginal, testLOriginal);

    string type = "sapo";
    map<string,string> p;
    SVMClassifier svm(type,p);

    cout << "Read features ok" << endl << "Training with " << posCountTrain << " pos and " << negCountTrain << " neg" << endl;
    svm.train(train,trainL);
    cout << "Training ok" << endl << "Testing with " << posCountTest << " pos and " << negCountTest << " neg" << endl;
    int tp = 0;
    int fn = 0;

    int tn = 0;
    int fp = 0;

    Mat classifications(test.rows,1,CV_32F);

    for(int i = 0; i < test.rows; i++){
        Mat features = test.row(i);

        float label = testL.at<float>(i,0);

        float detected = svm.classify(features);

        //cout << label << " " << detected << endl;

        if (label == detected && label == 1.0f)
            tp++;
        else if (label == detected && label == 0.0f)
            tn++;
        else if (label != detected && label == 1.0f)
            fn++;
        else if (label != detected && label == 0.0f)
            fp++;

        classifications.at<float>(i,0) = detected;
    }
    int correct = tp+tn;
    int wrong = fn+fp;

    cout << "keyframes: " << (correct/double(correct+wrong))*100 << " %: tp: " << tp << " tn: " << tn << " fp: " << fp  <<  " fn: " << fn << endl;

    int lastVideoId = -1;
    int totalVideoFrames = 0;
    int currentVideoFramesPositives = 0;

    tp = 0;
    tn = 0;
    fp = 0;
    fn = 0;

    stringstream ss;

    int error = 0;
    float label;
    float classification;

    for(int i = 0; i < test.rows; i++){
        float videoId = testLOriginal.at<float>(i,1);

        if (lastVideoId != videoId && i != 0){
            double ratio = currentVideoFramesPositives/(float)totalVideoFrames;

            if (ratio >= 0.5){ //concept was detected in the video
                if(label == 1)
                    tp++;
                else if (label == 0)
                    fp++;
                else {
                    error++;
                }
            } else {
                if(label == 1)
                    fn++;
                else if (label == 0)
                    tn++;
                else {
                    error++;
                }
            }
            if(label != 0 && label != 1)
                cout << "error: line: " << i << " label: " << label << endl;
            ss << lastVideoId << ";" << label << ";" << ratio << endl;
            currentVideoFramesPositives = 0;
            totalVideoFrames = 0;
        }

        label = testL.at<float>(i,0);
        classification = classifications.at<float>(i,0);

        //if classification is positive, increment
        if (classification == 1)
            currentVideoFramesPositives++;
        totalVideoFrames++;

        lastVideoId = videoId;
    }


        double ratio = currentVideoFramesPositives/(float)totalVideoFrames;

            if (ratio >= 0.5){ //concept was detected in the video
                if(label == 1)
                    tp++;
                else if (label == 0)
                    fp++;
                else {
                    cout << "error: " << label << endl;
                    error++;
                }
            } else {
                if(label == 1)
                    fn++;
                else if (label == 0)
                    tn++;
                else {
                    cout << "error: " << label << endl;
                    error++;
                }

            }
            if(label != 0 && label != 1)
                cout << "error: line: last label: " << label << endl;

    correct = tp+tn;
    wrong = fn+fp;

    cout << "video: " << (correct/double(correct+wrong))*100 << " %: tp: " << tp << " tn: " << tn << " fp: " << fp  <<  " fn: " << fn << " err: " << error << endl;

    cout << "Test ok" << endl;

    cout << ss.str();


    return 0;
}


int main(int argc, char *argv[])
{
	//testLoadSaveIClassifier(argc, argv);
	//testLoadSaveIIndexer(argc, argv);
	//faceDetectionParameterChallenge(argc, argv);
    //testAllClassifiersBin(argc, argv);
    //createMedCatClassifier(argc, argv);
    //testDatabaseConnection(argc, argv);
    //classifyAllImagesCondor(argc, argv);

    //extractAllFeaturesCKv2(argc, argv);
    //merger(argc, argv);
    //extractAllFeaturesCK(argc, argv);
	//testMSIDXIndexer(argc, argv);

    //classifyAllBlipImagesCondor(argc, argv);
    //classifyAllBlipImagesCondor(argc, argv);

    classifySapoAllVideos(argc, argv);
	//createBlipKnnVWDict(argc, argv);
    return 0;
}
