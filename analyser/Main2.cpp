#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <memory>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <time.h>
//#include <math>


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <db_cxx.h>

#include "nTag/SRClassifier.h"
#include "nVector/SRExtractor.h"
#include "nVector/ANdOMPExtractor.h"
#include "../indexer/SRIndexer.h"


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

#include "nVector/GISTExtractor.h"




#include "nVector/LLCExtractor.h"



#include "nRoi/FaceDetection.h"

#include "nTag/kNNClassifier.h"
#include "nTag/SVMClassifier.h"
#include "nTag/VWBasicClassifier.h"

#include "tools/TrainTestFeaturesTools.h"
#include "tools/MIRFlickrImporter.h"
#include "tools/tinyImageImporter.h"
#include "tools/oneBillionImporter.h"
#include "tools/FrameFilter.h"

#include "FactoryAnalyser.h"

#include "../indexer/FactoryIndexer.h"
#include "../indexer/IIndexer.h"
#include "../indexer/FlannkNNIndexer.h"
#include "../indexer/MSIDXIndexer.h"
#include "../indexer/DistributedIndexWrapperServer.h"
#include "../indexer/DistributedIndexWrapperClient.h"
#include "../indexer/DistributedIndexWrapperClientDist.h"



#include "../commons/StringTools.h"

#include "../dataModel/DatabaseConnection.h"

#include "../commons/LoadConfig.h"

#include "../rest/Endpoints/analyser/ExtractFeatures.h"
#include "../rest/RestServer.h"


#include <ksvd/clustering.h>

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

		cv::Mat labels(1,2,CV_32F);
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

		cv::Mat features;
		cv::Mat labels;

		MatrixTools::readBin(file, features, labels);

		TrainTestFeaturesTools ttft(features, labels,classi);

		cout << ttft.crossValidateAll(divisions) << endl;
	} else { //separated files

		string file(argv[1]);
		string fileTest(argv[2]);


		cv::Mat features;
		cv::Mat labels;

		cv::Mat testFeatures;
		cv::Mat testLabels;

		MatrixTools::readBin(file, features, labels);
		MatrixTools::readBin(fileTest, testFeatures, testLabels);

		TrainTestFeaturesTools ttft(features, labels, testFeatures, testLabels,classi);
		cout << ttft.testAll() << endl;
	}


	return 0;
}

void testLoadSaveIClassifier(int argc, char *argv[]){

	string file(argv[1]);

	cv::Mat features;
	cv::Mat labels;

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

	cv::Mat features;
	cv::Mat labels;

	MatrixTools::readBin(file, features, labels);
	IIndexer* vw = new FlannkNNIndexer();

	vw->index(features);
	vw->save("medicalImage_CEDD_kNN");
	cv::Mat q = features.row(0);

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

	cv::Mat src;

	int u = 0;
	int step = 100;
	int size = is.getImageCount();
	double lastT = cvGetTickCount();
	double startT = cvGetTickCount();

	int facesDetected = 0;


	//int angles[] = {-20,20};
	//int anglesLen = 2;
	while (!(src = is.nextImage()).empty()) {

		vector<cv::Mat> faceImages;
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
		cv::Mat newSrc;

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

	cv::Mat features;
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
	cv::Mat q = features.row(0);

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

	cv::Mat t1;
	cv::Mat t2;

	cv::Mat l1;
	cv::Mat l2;

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

	cv::Mat src;

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

    cv::Mat features, labels;

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

        cv::Mat f1, f2, comb;
        cv::Mat label(1,1,CV_32F);

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

		cv::Mat labels(1,2,CV_32F);
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

	cv::Mat src;

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

    cv::Mat features, labels;

    while (j < is.getImageCount()) {
        src = is.nextImage();
        //cout << is.getImagePath() << endl;

        //cout <<  StringTools::split(is.getCurrentImageInfoField(0),'.')[0] << endl;
        if (!src.empty()){
        string iri = StringTools::split(is.getCurrentImageInfoField(0),'.')[0];


        if (j % 100 == 0)
            cout << j << " " << is.getImageCount() << endl;

        cv::Mat f1, f2, comb;
        cv::Mat label(1,1,CV_32F);

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

	cv::Mat src;

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

    cv::Mat features, labels;

    for (int j = 0; j < imagesToProcess; j++) {
        try{
        src = is.nextImage();
        //cout << is.getImagePath() << endl;

        //cout <<  StringTools::split(is.getCurrentImageInfoField(0),'.')[0] << endl;
        if (!src.empty()){
        string iri = StringTools::split(is.getCurrentImageInfoField(0),'.')[0];

        cv::Mat f1, f2, comb;
        cv::Mat label(1,1,CV_32F);

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

	cv::Mat src;


	cv::Mat vocabulary, labelsAll;
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



            	cv::Mat labels(1,2,CV_32F);

                labels.at<float>(0,0) = totalImages + startAt;
                labels.at<float>(0,1) = totalImages;


                totalImages++;
                for(uint i = 0; i < fExtractors.size(); i++){
                    cv::Mat features;
                    fExtractors.at(i)->extractFeatures(src,features);
                	stringstream ss;
			        ss << "blip_" << fExtractors.at(i)->getName() << "_";
			        ss << std::setw(2) << std::setfill('0') << myDivision << ".bin";
			        string filename = ss.str();
			        MatrixTools::writeBinV2(filename,features,labels,true);
			        features.release();
                }


                for(uint i = 0; i < kExtractors.size(); i++){
                    cv::Mat features;
                    vector<KeyPoint> keypoints;
                    kExtractors.at(i)->extractFeatures(src,keypoints,features);

                    stringstream ss;
        			ss << "blip_" << kExtractors.at(i)->getName() << "_";
        			ss << std::setw(2) << std::setfill('0') << myDivision << ".bin";
        			string filename = ss.str();
        			//MatrixTools::writeBinV3(filename,features,keypoints,labels,true);
        			features.release();
        			//if (kExtractors.at(i)->getName() == "SIFTExtractor"){
        				cv::Mat imgDescriptor;
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
    cv::Mat descriptors;

    cv::Mat originalGrayImage = imread(parameters["filename"]);

    detector->detect(originalGrayImage, keypoints);
    extractor->compute(originalGrayImage, keypoints, descriptors);

    cv::Mat label(1,2,CV_32F);

    label.at<float>(0,0) = descriptors.cols;
    label.at<float>(0,1) = descriptors.cols;

    vector<cv::Mat> descriptorsMat;
    descriptorsMat.push_back(descriptors);

    vector<vector<KeyPoint> > keypointsVec;
    keypointsVec.push_back(keypoints);

    string s = parameters["outfilename"];

    MatrixTools::writeBinV2(s,descriptorsMat,label,true);
    //MatrixTools::writeBinV3(s,descriptorsMat,keypointsVec,label,true);

    cout << descriptorsMat.at(0).row(0).colRange(0,2) << " " << descriptorsMat.at(0).rows << " " << label.cols << " " << label.rows << " " << keypointsVec.at(0).at(0).pt.x << " " << keypointsVec.at(0).at(1).pt.y << endl;

    vector<cv::Mat> descriptorsMat2;
    cv::Mat label2;
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

	cv::Mat vocabulary, labelsAll;
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

	cv::Mat imgDescriptor;
	vector<KeyPoint> keypoints2;
	cv::Mat image = imread("/home/amourao/code/searchservices/bush.jpg");
	detector->detect(image, keypoints2);

	cout << "extracting demo image" << endl;
	bowMatcher.compute(image, keypoints2, imgDescriptor);

	cv::Mat tempLabels;


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
    string paramFile(argv[6]);

	LoadConfig::load(paramFile,parameters);

    map<string,string> params;

    params["input"] = string(argv[1]);
	params["analyser"] = string(argv[2]);
	params["task"] = string(argv[3]);
	params["output"] = string(argv[4]);
	params["filter"] = string(argv[5]);

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

    cv::Mat featuresNegTrain;
    cv::Mat featuresNegTrainL;
    cv::Mat featuresNegTest;
    cv::Mat featuresNegTestL;

    //get positive examples
    //number of examples is strictly based on the ratios provided as parameters

    cv::Mat featuresPos, labelsPos;
    MatrixTools::readBin(positiveExamples,featuresPos,labelsPos);

    int posCountTrain = featuresPos.rows*splitTrainPos;
    int posCountTest = featuresPos.rows*splitTestPos;

    cv::Mat featurePosTrain = featuresPos.rowRange(0,posCountTrain);
    cv::Mat featuresPosTest = featuresPos.rowRange(posCountTrain,posCountTrain+posCountTest);

    cv::Mat featurePosTrainL = cv::Mat(posCountTrain,1, CV_32F, 1.0);
    cv::Mat featuresPosTestL = cv::Mat(posCountTest,1, CV_32F, 1.0);

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
        cv::Mat featuresS, labelsS;
        MatrixTools::readBin(negExample,featuresS,labelsS);

        int currSplitTrain = negCountTrainExpectedPerClass;
        int currSplitTest = negCountTestExpectedPerClass;

        //if the number of features required is too big, split all data using the same train/test ratio
        if((currSplitTrain+currSplitTest)> featuresS.rows){
            currSplitTrain = featuresS.rows * negCountTrainExpected/(negCountTestExpected+negCountTrainExpected);
            currSplitTest = featuresS.rows - currSplitTrain;
        }

        cv::Mat featuresNegTrainC = featuresS.rowRange(0,currSplitTrain);
        cv::Mat featuresNegTestC = featuresS.rowRange(currSplitTrain,currSplitTrain+currSplitTest);

        cv::Mat featuresNegTrainLC = cv::Mat(currSplitTrain,1, CV_32F, 0.0);
        cv::Mat featuresNegTestLC = cv::Mat(currSplitTest,1, CV_32F, 0.0);

        featuresNegTrain.push_back(featuresNegTrainC);
        featuresNegTrainL.push_back(featuresNegTrainLC);

        featuresNegTest.push_back(featuresNegTestC);
        featuresNegTestL.push_back(featuresNegTestLC);

        negCountTrain+=currSplitTrain;
        negCountTest+=currSplitTest;
    }




    cv::Mat train;
    cv::Mat trainL;
    cv::Mat test;
    cv::Mat testL;

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
        cv::Mat features = test.row(i);

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


int extractOnlyMiddleFrames(cv::Mat& featurePosTrain, cv::Mat& featurePosTrainL, cv::Mat& featurePosTrainLOriginal){
        cv::Mat tmpfeaturePosTrain;
        cv::Mat tmpfeaturePosTrainL;
        cv::Mat tmpfeaturePosTrainLOriginal;

        featurePosTrain.copyTo(tmpfeaturePosTrain);
        featurePosTrainL.copyTo(tmpfeaturePosTrainL);
        featurePosTrainLOriginal.copyTo(tmpfeaturePosTrainLOriginal);

        featurePosTrain = cv::Mat();
        featurePosTrainL = cv::Mat();
        featurePosTrainLOriginal = cv::Mat();

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

       return featurePosTrain.rows;
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

    cv::Mat featuresNegTrain;
    cv::Mat featuresNegTrainL;
    cv::Mat featuresNegTrainLOriginal;
    cv::Mat featuresNegTest;
    cv::Mat featuresNegTestL;
    cv::Mat featuresNegTestLOriginal;

    //get positive examples
    //number of examples is strictly based on the ratios provided as parameters

    cv::Mat featuresPos, labelsPos;
    MatrixTools::readBin(positiveExamples,featuresPos,labelsPos);

    int posCountTrain = featuresPos.rows*splitTrainPos;
    int posCountTest = featuresPos.rows*splitTestPos;

    int lastVideo = labelsPos.at<float>(posCountTrain,1);

    //go until last frame of the current video to avoid spliting videos across train/test data
    while(labelsPos.at<float>(++posCountTrain,1) == lastVideo)
        ;

    int lastTestValue = min(posCountTrain+posCountTest,labelsPos.rows);
    lastVideo = labelsPos.at<float>(lastTestValue,1);
    //go until last frame of the current video to avoid spliting videos across train/test data
    while(lastTestValue < labelsPos.rows && labelsPos.at<float>(++lastTestValue,1) == lastVideo)
        ;

    lastTestValue = min(lastTestValue,labelsPos.rows);

    posCountTest = lastTestValue-posCountTrain;

    cv::Mat featurePosTrain = featuresPos.rowRange(0,posCountTrain);
    cv::Mat featurePosTrainLOriginal = labelsPos.rowRange(0,posCountTrain);
    cv::Mat featuresPosTestLOriginal = labelsPos.rowRange(posCountTrain,lastTestValue);
    cv::Mat featuresPosTest = featuresPos.rowRange(posCountTrain,lastTestValue);

    cv::Mat featurePosTrainL= cv::Mat::ones(posCountTrain,1, CV_32F);
    cv::Mat featuresPosTestL= cv::Mat::ones(featuresPosTest.rows,1, CV_32F);


    //option to select only frames from the middle of the scene
    if(onlyMiddleKeyframes){

        //cout << "Using only middle keyframes positive examples" << endl;

        posCountTrain = extractOnlyMiddleFrames(featurePosTrain,featurePosTrainL,featurePosTrainLOriginal);
        posCountTest = extractOnlyMiddleFrames(featuresPosTest,featuresPosTestL,featuresPosTestLOriginal);

        splitTrainNeg*=2;
        splitTestNeg*=2;
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
        cv::Mat featuresS, labelsS;
        MatrixTools::readBin(negExample,featuresS,labelsS);

        if(labelsS.cols == 8){
            labelsS.col(2).copyTo(labelsS.col(1));
            labelsS = labelsS.colRange(0,5);
        }

        int currSplitTrain = negCountTrainExpectedPerClass;
        int currSplitTest = negCountTestExpectedPerClass;

        //if the number of features required is too big, split all data using the same train/test ratio
        if((currSplitTrain+currSplitTest)> featuresS.rows){
            currSplitTrain = featuresS.rows * (negCountTrainExpected/(float)(negCountTestExpected+negCountTrainExpected));
            currSplitTest = featuresS.rows - currSplitTrain;
        }

        lastVideo = labelsS.at<float>(currSplitTrain,1);

        //go until last frame of the current video to avoid spliting videos across train/test data
        while(labelsS.at<float>(++currSplitTrain,1) == lastVideo)
            ;

        lastTestValue = min(currSplitTrain+currSplitTest,labelsS.rows);
        lastVideo = labelsS.at<float>(lastTestValue,1);

        //go until last frame of the current video to avoid spliting videos across train/test data
        while(lastTestValue++ < labelsS.rows && labelsS.at<float>(lastTestValue,1) == lastVideo)
            ;
        lastTestValue = min(lastTestValue,labelsS.rows);

        cv::Mat featuresNegTrainC = featuresS.rowRange(0,currSplitTrain);
        cv::Mat featuresNegTrainLCOriginal = labelsS.rowRange(0,currSplitTrain);
        cv::Mat featuresNegTestC = featuresS.rowRange(currSplitTrain,lastTestValue);
        cv::Mat featuresNegTestLCOriginal = labelsS.rowRange(currSplitTrain,lastTestValue);

        currSplitTest = featuresNegTestC.rows;

        cv::Mat featuresNegTrainLC = cv::Mat::zeros(currSplitTrain,1, CV_32F);
        cv::Mat featuresNegTestLC = cv::Mat::zeros(currSplitTest,1, CV_32F);

        featuresNegTrain.push_back(featuresNegTrainC);
        featuresNegTrainL.push_back(featuresNegTrainLC);
        featuresNegTrainLOriginal.push_back(featuresNegTrainLCOriginal);

        featuresNegTest.push_back(featuresNegTestC);
        featuresNegTestL.push_back(featuresNegTestLC);
        featuresNegTestLOriginal.push_back(featuresNegTestLCOriginal);

        negCountTrain+=currSplitTrain;
        negCountTest+=currSplitTest;
    }


    if(onlyMiddleKeyframes){
        negCountTrain = extractOnlyMiddleFrames(featuresNegTrain,featuresNegTrainL,featuresNegTrainLOriginal);
        negCountTest = extractOnlyMiddleFrames(featuresNegTest,featuresNegTestL,featuresNegTestLOriginal);
    }

    cv::Mat train;
    cv::Mat trainL;
    cv::Mat trainLOriginal;
    cv::Mat test;
    cv::Mat testL;
    cv::Mat testLOriginal;

    vconcat(featurePosTrain, featuresNegTrain, train);
    vconcat(featurePosTrainL, featuresNegTrainL, trainL);
    vconcat(featurePosTrainLOriginal, featuresNegTrainLOriginal, trainLOriginal);
    vconcat(featuresPosTest, featuresNegTest, test);
    vconcat(featuresPosTestL, featuresNegTestL, testL);
    vconcat(featuresPosTestLOriginal, featuresNegTestLOriginal, testLOriginal);

    string type = "sapo";
    map<string,string> p;
    SVMClassifier svm(type,p);

    cout << posCountTrain << ";" << negCountTrain << endl;

    stringstream modelSS;
    vector<string> trainNameV = StringTools::split(positiveExamples,'/');
    trainNameV = StringTools::split(trainNameV[trainNameV.size()-1],'.');

    modelSS << type << "_" << trainNameV[0];
    string trainName = modelSS.str();

    if(!svm.load(trainName)){
        svm.train(train,trainL);
        svm.save(trainName);
    }

    cout << posCountTest << ";" << negCountTest << endl << endl;
    int tp = 0;
    int fn = 0;

    int tn = 0;
    int fp = 0;

    const vector<double> ratiosk({-1,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,-0.2,-0.1,-0.05,-0.02,0,0.02,0.05,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1});
    vector<int> tpk(ratiosk.size(),0);
    vector<int> tnk(ratiosk.size(),0);
    vector<int> fpk(ratiosk.size(),0);
    vector<int> fnk(ratiosk.size(),0);

    cv::Mat classifications(test.rows,1,CV_32F);
    cv::Mat classificationsScore(test.rows,1,CV_32F);

    for(int i = 0; i < test.rows; i++){
        cv::Mat features = test.row(i);

        float label = testL.at<float>(i,0);

        float prediction = svm.getClassificationConfidence(features);
        float detected = prediction < 0;

        //cout << label << " " << detected << endl;

        if (label == detected && label == 1.0f)
            tp++;
        else if (label == detected && label == 0.0f)
            tn++;
        else if (label != detected && label == 1.0f)
            fn++;
        else if (label != detected && label == 0.0f)
            fp++;

            double ratio = -prediction;
            for (uint j = 0; j < ratiosk.size(); j++){

                if (ratio >= ratiosk.at(j)){
                    if(label == 1)
                        tpk.at(j)++;
                    else if (label == 0)
                        fpk.at(j)++;
                } else {
                    if(label == 1)
                        fnk.at(j)++;
                    else if (label == 0)
                        tnk.at(j)++;
                }
            }

        classifications.at<float>(i,0) = detected;
        classificationsScore.at<float>(i,0) = ratio;
    }

    cout << "Keyframe Binary" << endl;
    cout << ";" << (tp + tn)/float(tp+tn+fp+fn) << ";" << (tp)/float(tp+fp) << ";" << (tp)/float(tp+fn) << ";;;"  << tp << ";" << tn << ";" << fp  <<  ";" << fn << endl << endl;

    cout << "Keyframe Thresholded" << endl;
    for (uint j = 0; j < ratiosk.size(); j++){
        cout << ratiosk.at(j) << ";" << (tpk.at(j) + tnk.at(j))/float(tpk.at(j)+tnk.at(j)+fpk.at(j)+fnk.at(j)) << ";" << (tpk.at(j))/float(tpk.at(j)+fpk.at(j)) << ";" << (tpk.at(j))/float(tpk.at(j)+fnk.at(j)) << ";;;" << tpk.at(j) << ";" << tnk.at(j) << ";" << fpk.at(j)  <<  ";" << fnk.at(j) << endl;
    }
    cout << endl;

    int lastVideoId = -1;
    int totalVideoFrames = 0;
    int currentVideoFramesPositives = 0;
    double currentVideoFramesScore = 0;

    stringstream ss;

    int error = 0;
    float label = -1;
    float classification;

    const vector<double> ratios({0,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.4,0.45,0.5,0.55,0.6,0.65,0.7,0.75,0.8,0.85,0.9,0.95,1});
    vector<int> tpv(ratios.size(),0);
    vector<int> tnv(ratios.size(),0);
    vector<int> fpv(ratios.size(),0);
    vector<int> fnv(ratios.size(),0);

    const vector<double> ratiosvalt({-1,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,-0.2,-0.1,-0.05,-0.02,0,0.02,0.05,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1});
    vector<int> tpvalt(ratiosvalt.size(),0);
    vector<int> tnvalt(ratiosvalt.size(),0);
    vector<int> fpvalt(ratiosvalt.size(),0);
    vector<int> fnvalt(ratiosvalt.size(),0);

    int totalVideos = 0;

    //fstream file("log.txt",fstream::out);

    for(int i = 0; i < test.rows; i++){
        float videoId;

        videoId = testLOriginal.at<float>(i,1);

        if (lastVideoId != videoId && i != 0){
            totalVideos++;
            double ratio = currentVideoFramesPositives/(float)totalVideoFrames;
            for (uint j = 0; j < ratios.size(); j++){

                if (ratio >= ratios.at(j)){ //concept was detected in the video
                    if(label == 1)
                        tpv.at(j)++;
                    else if (label == 0)
                        fpv.at(j)++;
                    else {
                        error++;
                    }
                } else {
                    if(label == 1)
                        fnv.at(j)++;
                    else if (label == 0)
                        tnv.at(j)++;
                    else {
                        error++;
                    }
                }
            }

            ratio = currentVideoFramesScore/(float)totalVideoFrames;
            for (uint j = 0; j < ratiosvalt.size(); j++){

                if (ratio >= ratiosvalt.at(j)){ //concept was detected in the video
                    if(label == 1)
                        tpvalt.at(j)++;
                    else if (label == 0)
                        fpvalt.at(j)++;
                    else {
                        error++;
                    }
                } else {
                    if(label == 1)
                        fnvalt.at(j)++;
                    else if (label == 0)
                        tnvalt.at(j)++;
                    else {
                        error++;
                    }
                }
            }
            //file << lastVideoId << ";" << label << ";" << ratio << endl;
            currentVideoFramesPositives = 0;
            currentVideoFramesScore = 0;
            totalVideoFrames = 0;
        }

        label = testL.at<float>(i,0);
        classification = classifications.at<float>(i,0);
        float classificationsScoreFrame = classificationsScore.at<float>(i,0);

        currentVideoFramesScore+=classificationsScoreFrame;
        //if classification is positive, increment
        if (classification == 1)
            currentVideoFramesPositives++;
        totalVideoFrames++;

        lastVideoId = videoId;
    }

    totalVideos++;
    double ratio = currentVideoFramesPositives/(float)totalVideoFrames;
    for (uint j = 0; j < ratios.size(); j++){
        if (ratio >= ratios.at(j)){ //concept was detected in the video
            if(label == 1)
                tpv.at(j)++;
            else if (label == 0)
                fpv.at(j)++;
            else {
                error++;
            }
        } else {
            if(label == 1)
                fnv.at(j)++;
            else if (label == 0)
                tnv.at(j)++;
            else {
                error++;
            }
        }
    }

    ratio = currentVideoFramesScore/(float)totalVideoFrames;
            for (uint j = 0; j < ratiosvalt.size(); j++){

                if (ratio >= ratiosvalt.at(j)){ //concept was detected in the video
                    if(label == 1)
                        tpvalt.at(j)++;
                    else if (label == 0)
                        fpvalt.at(j)++;
                    else {
                        error++;
                    }
                } else {
                    if(label == 1)
                        fnvalt.at(j)++;
                    else if (label == 0)
                        tnvalt.at(j)++;
                    else {
                        error++;
                    }
                }
            }

    if(label != 0 && label != 1)
        cout << "error: line: last label: " << label << endl;
    //file << lastVideoId << ";" << label << ";" << ratio << endl;

    cout << "Video Thresholded \"Keyframe Binary\"" << endl;
    for (uint j = 0; j < ratios.size(); j++){
        cout << ratios.at(j) << ";" << (tpv.at(j) + tnv.at(j))/float(tpv.at(j)+tnv.at(j)+fpv.at(j)+fnv.at(j)) << ";" << (tpv.at(j))/float(tpv.at(j)+fpv.at(j))  << ";" << (tpv.at(j))/float(tpv.at(j)+fnv.at(j)) << ";;;" << tpv.at(j) << ";" << tnv.at(j) << ";" << fpv.at(j)  <<  ";" << fnv.at(j) << endl;
    }
    cout << endl;
    cout << "Video Thresholded \"Keyframe Thresholded \"" << endl;
    for (uint j = 0; j < ratiosvalt.size(); j++){
        cout << ratiosvalt.at(j) << ";" << (tpvalt.at(j) + tnvalt.at(j))/float(tpvalt.at(j)+tnvalt.at(j)+fpvalt.at(j)+fnvalt.at(j)) << ";" << (tpvalt.at(j))/float(tpvalt.at(j)+fpvalt.at(j)) << ";" << (tpvalt.at(j))/float(tpvalt.at(j)+fnvalt.at(j)) << ";;;" << tpvalt.at(j) << ";" << tnvalt.at(j) << ";" << fpvalt.at(j)  <<  ";" << fnvalt.at(j) << endl;
    }

    /*
    for (int i = 0; i < trainLOriginal.rows; i++){
        file << trainL.row(i) << " " << trainLOriginal.row(i) << endl;
    }
    file << endl << endl;
    for (int i = 0; i < testLOriginal.rows; i++){
        file << testL.row(i) << " " << testLOriginal.row(i) << endl;
    }
    */
    return 0;
}

int extractAndSaveToBerkeleyDB(int argc, char *argv[]){

    map<string,string> parameters;

    string textFile = string(argv[1]);
    TextFileSourceV2 is(textFile);

    string features = string(argv[2]);
    string dbBasePath = string(argv[3]);

    string config = string(argv[4]);
    LoadConfig::load(config,parameters);

    vector<string> featuresArr = StringTools::split(features,',');
    vector<IAnalyser*> analysers;

    FactoryAnalyser * f = FactoryAnalyser::getInstance();

    for(uint i = 0; i < featuresArr.size(); i++){
        IAnalyser* analyser = (IAnalyser*)f->createType(featuresArr.at(i));

        if(analyser == NULL){
            cout << featuresArr.at(i) << " not found." << endl;
            continue;
        }
        analysers.push_back(analyser);



    }
    cv::Mat src;

    for (int i = 0; i < is.getImageCount(); i++) {
		if (!(src = is.nextImage()).empty()) { // src contains the image, but the IAnalyser interface needs a path

            string id1, id2, pmcid, imageid, path;

            path = is.getImagePath();

			stringstream liness(is.getImageInfo());
            getline(liness, id1, ';');
			getline(liness, id2, ';');
			getline(liness, pmcid, ';');
			getline(liness, imageid);
			imageid.erase(remove(imageid.begin(), imageid.end(), '\r'), imageid.end());
			imageid.erase(remove(imageid.begin(), imageid.end(), '\n'), imageid.end());


            for(uint j = 0; j < analysers.size(); j++){
                    IAnalyser* analyser = analysers.at(j);

                    string keyString = pmcid + ";" + imageid;
                    vector<float>* featureVec = (vector<float>*)analyser->getFeatures(path)->getValue();


                    string dbPath = dbBasePath + "_" + featuresArr.at(j) + ".db";
                    Db db(NULL, 0);               // Instantiate the Db object
                    u_int32_t oFlags = DB_CREATE; // Open flags;
                    try {
                        // Open the database
                        db.open(NULL,               // Transaction pointer
                                dbPath.c_str(),             // Database file name
                                NULL,               // Optional logical database name
                                DB_BTREE,           // Database access method
                                oFlags,             // Open flags
                                0);                 // File mode (using defaults)
                    // DbException is not subclassed from std::exception, so
                    // need to catch both of these.
                    } catch(DbException &e) {
                        // Error handling code goes here
                    } catch(std::exception &e) {
                        // Error handling code goes here
                    }

                    //cout << featureVec->size() << endl;
                    //for(int k = 0; k < featureVec->size(); k++){
                    //    cout << featureVec->at(k) << " ";
                    //}
                    //cout << endl;

                    Dbt key(&keyString[0], keyString.size());
                    Dbt data(&featureVec->at(0), sizeof(float)*featureVec->size());

                    //int ret = db.put(NULL, &key, &data, DB_NOOVERWRITE);
                    //if (ret == DB_KEYEXIST)
                        //cout << "Put failed because key already exists: " << keyString << endl;

                    int descSize = featureVec->size();
                    string descSizeName = "size";

                    Dbt keyS(&descSizeName[0], descSizeName.size());
                    Dbt dataS(&descSize, sizeof(int));
                    ret = db.put(NULL, &keyS, &dataS, DB_NOOVERWRITE);
                    //if (ret == DB_KEYEXIST)
                    //    cout << "Put failed because key already exists: " << descSizeName << endl;



                    try {
                        // Close the database
                        db.close(0);
                        // DbException is not subclassed from std::exception, so
                        // need to catch both of these.
                    } catch(DbException &e) {
                        // Error handling code goes here
                    } catch(std::exception &e) {
                        // Error handling code goes here
                    }





            }
		}
    }

    return 0;
}



int readBerkeleyDB (int argc, char *argv[]){

    string features = string(argv[2]);
    string dbBasePath = string(argv[3]);


    string textFile = string(argv[1]);
    TextFileSourceV2 is(textFile);


    vector<string> featuresArr = StringTools::split(features,',');

    cv::Mat src;
    for (int i = 0; i < is.getImageCount(); i++) {
		if (!(src = is.nextImage()).empty()) { // src contains the image, but the IAnalyser interface needs a path

            string id1, id2, pmcid, imageid, path;

            path = is.getImagePath();

			stringstream liness(is.getImageInfo());
            getline(liness, id1, ';');
			getline(liness, id2, ';');
			getline(liness, pmcid, ';');
			getline(liness, imageid);
			imageid.erase(remove(imageid.begin(), imageid.end(), '\r'), imageid.end());
			imageid.erase(remove(imageid.begin(), imageid.end(), '\n'), imageid.end());


            for(uint i = 0; i < featuresArr.size(); i++){
                string dbPath = dbBasePath + "_" + featuresArr.at(i) + ".db";
                Db db(NULL, 0);               // Instantiate the Db object
                u_int32_t oFlags = DB_CREATE; // Open flags;
                try {
                    // Open the database
                    db.open(NULL,               // Transaction pointer
                            dbPath.c_str(),             // Database file name
                            NULL,               // Optional logical database name
                            DB_BTREE,           // Database access method
                            oFlags,             // Open flags
                            0);                 // File mode (using defaults)
                // DbException is not subclassed from std::exception, so
                // need to catch both of these.
                } catch(DbException &e) {
                    // Error handling code goes here
                } catch(std::exception &e) {
                    // Error handling code goes here
                }

                int size = 0;
                Dbt keySizeDbt, dataSizeDbt;
                string keySizeString = "size";
                keySizeDbt.set_data(&keySizeString[0]);
                keySizeDbt.set_size(keySizeString.size());

                dataSizeDbt.set_data(&size);
                dataSizeDbt.set_ulen(sizeof(int));
                dataSizeDbt.set_flags(DB_DBT_USERMEM);

                //int ret = db.get(NULL, &keySizeDbt, &dataSizeDbt, 0);

                cout << size << endl;

                float* data = new float[size];

                Dbt keyDbt, dataDbt;
                string keyString = pmcid + ";" + imageid;
                keyDbt.set_data(&keyString[0]);
                keyDbt.set_size(keyString.size());

                dataDbt.set_data(&data[0]);
                dataDbt.set_ulen(sizeof(float)*size);
                dataDbt.set_flags(DB_DBT_USERMEM);

                db.get(NULL, &keyDbt, &dataDbt, 0);


                for(int j = 0; j < size; j++){
                    cout << data[j] << " ";
                }
                cout << endl;


                try {
                    // Close the database
                    db.close(0);
                    // DbException is not subclassed from std::exception, so
                    // need to catch both of these.
                } catch(DbException &e) {
                    // Error handling code goes here
                } catch(std::exception &e) {
                    // Error handling code goes here
                }



            }
		}
    }



    return 0;
}

int playground(int argc, char *argv[]){
    /*
    map<string,string> params;

	params["dictPath"] = "/home/amourao/code/matlab/CVPR10-LLC/B.bin";
	params["knn"] = "5";
	params["beta"] = "0.0001";

    arma::mat A;
	A.load("/home/amourao/code/matlab/CVPR10-LLC/A.mat",arma::raw_ascii);
	A.save("/home/amourao/code/matlab/CVPR10-LLC/A.bin");

	arma::mat B;
	B.load("/home/amourao/code/matlab/CVPR10-LLC/B.mat",arma::raw_ascii);
	B.save("/home/amourao/code/matlab/CVPR10-LLC/B.bin");

	arma::mat X;
	X.load("/home/amourao/code/matlab/CVPR10-LLC/X.mat",arma::raw_ascii);
	X.save("/home/amourao/code/matlab/CVPR10-LLC/X.bin");

	arma::mat R;
	R.load("/home/amourao/code/matlab/CVPR10-LLC/R.mat",arma::raw_ascii);
	R.save("/home/amourao/code/matlab/CVPR10-LLC/R.bin");

    string t = "a";

    arma::mat R2;
    LLCExtractor llc(t,params);


    llc.extractFeatures(X,R2);

    cout <<  accu(abs(R)) << endl;
    cout <<  accu(abs(R*B)) << endl;

    cout <<  accu(abs(R2)) << endl;
    cout <<  accu(abs(R2*B)) << endl;

    cout <<  accu(abs(R*B-A.t())) << endl;
    cout <<  accu(abs(R2*B-A.t())) << endl;

    */

    /*
    cv::Mat dst1, dst2;
    string filename = "/home/amourao/Downloads/google-earth-view-1102.jpg";

    vector<int> ops;
    ops.push_back(8);
    ops.push_back(8);
    ops.push_back(4);
    GISTExtractor gist(4,3,ops,320,320);

    cv::Mat src = imread(filename);
    gist.extractFeatures(src,dst1);

    cout << dst1.rows << " " << dst1.cols << endl;

    SRExtractor sr;
    sr.extractFeatures(dst1,dst2);
    cout << dst2 << endl;





    map<string, string> parameters;

    vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IEndpoint*> endpoints;
	vector<IClassifier*> classifiers;

    string paramFile(argv[1]);
	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

    string featuresType = "lbp885";
    FactoryAnalyser * f = FactoryAnalyser::getInstance();
	FeatureExtractor* extractor = (FeatureExtractor*)f->createType(featuresType);

    vector<string> images;

    images.push_back("/home/amourao/tmp/IMG_6628.jpg");
    images.push_back("/home/amourao/tmp/IMG_6646.jpg");
    images.push_back("/home/amourao/tmp/IMG_6650.jpg");
    images.push_back("/home/amourao/tmp/IMG_6656.jpg");
    images.push_back("/home/amourao/tmp/IMG_6658.jpg");
    images.push_back("/home/amourao/tmp/IMG_6676.jpg");
    images.push_back("/home/amourao/tmp/IMG_6682.jpg");
    images.push_back("/home/amourao/tmp/IMG_6685.jpg");
    images.push_back("/home/amourao/tmp/IMG_6689.jpg");
    images.push_back("/home/amourao/tmp/IMG_6693.jpg");
    images.push_back("/home/amourao/tmp/IMG_6700.jpg");
    images.push_back("/home/amourao/tmp/IMG_6702.jpg");

    string indexerType = "DistributedIndexWrapperClientDistA";
    FactoryIndexer * fi = FactoryIndexer::getInstance();
	IIndexer* client = (IIndexer*)fi->createType(indexerType);

    for(int i = 0; i < images.size(); i++){
        string filename = images.at(i);
        cv::Mat o = imread(filename);
        cv::Mat src;
        extractor->extractFeatures(o,src);

        std::pair<vector<float>,vector<float> > out = client->knnSearchId(src,10,0.1);

        for(int i = 0; i < out.first.size(); i++){
            cout << out.first[i] << "\t" << out.second[i] << endl;
        }
    }

    indexerType = "DistributedIndexWrapperClientDistB";
    fi = FactoryIndexer::getInstance();
	client = (IIndexer*)fi->createType(indexerType);

    for(int i = 0; i < images.size(); i++){
        string filename = images.at(i);
        cv::Mat o = imread(filename);
        cv::Mat src;
        cerr << extractor << " " << o.colRange(0,3).rowRange(0,3) << endl;
        extractor->extractFeatures(o,src);

        std::pair<vector<float>,vector<float> > out = client->knnSearchId(src,10,0.1);

        for(int i = 0; i < out.first.size(); i++){
            cout << out.first[i] << "\t" << out.second[i] << endl;
        }
    }




    float* arrayData = (float*)src.data;

    float action = 1;
	float rows = src.rows;
	float cols = src.cols;

	vector<float> tmp;
	tmp.push_back(action);
	tmp.push_back(rows);
	tmp.push_back(cols);
	tmp.insert(tmp.end(), &arrayData[0], &arrayData[(int)(rows*cols)]);

    Poco::Net::SocketAddress server_address("localhost", 12345);
    Poco::Net::SocketAddress client_address("localhost", 12346);
    Poco::Net::DatagramSocket dgs(client_address);

    cout << dgs.sendTo(&tmp[0], tmp.size()*sizeof(float), server_address) << endl;

    char* buffer = new char[1024];

    int inBuffer = dgs.receiveBytes(buffer,1024);
    cout << inBuffer << endl;

    float* floatBuffer = reinterpret_cast<float*>(buffer);

    for(int i = 0; i < inBuffer/sizeof(float); i++){
        cout << floatBuffer[i] << ",";
	}
	cout << endl;
    */
    return 0;
}



int trainKSVD(int argc, char *argv[]){

    string paramFile(argv[1]);

	map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);


    int dimensionality = std::stoi(parameters["dimensionality"]);
    string type = "a";
    map<string,string> params;

	params["dimensions"] = std::to_string(dimensionality);
    params["iters"] = "25";

	params["eps"] = "1e-7";
	params["max_iters"] = "10";
	params["eps_ksvd"] = "1e-7";
	params["max_iters_ksvd"] = "10";

	params["normalize"] = "cols";

    arma::fmat T,VI,VQ;
    T.load(parameters["T"]);
    VI.load(parameters["VI"]);
    VQ.load(parameters["VQ"]);

    arma::fmat means = arma::mean(T);
    arma::fmat stddevs = arma::stddev(T);

    for(uint i = 0; i < T.n_cols; i++){
        if(stddevs.at(0,i) == 0)
            T.col(i) = (T.col(i) - means.at(0,i));
        else
            T.col(i) = (T.col(i) - means.at(0,i))/stddevs.at(0,i);
    }

    //oneBillionImporter importer;
    //importer.readBin("/localstore/amourao/saIndexingSplits/configCondor4_1_k.json_train.mat",1000,T,0);
    //importer.readBin("/media/Share/data/1-billion-vectors/siftsmall/siftsmall_learn.fvecs",1000,VI,1000);
    //importer.readBin("/media/Share/data/1-billion-vectors/siftsmall/siftsmall_learn.fvecs",100,VQ,1000+100);

    arma::fmat kmeans = sparse::kmeans(T, dimensionality, 100);
    kmeans.save(params["kmeansOutput"] /*"/home/amourao/code/searchservices/indexer/data/configCondor4_1_k_dict_kmeans.mat")*/);

    SRIndexer sr(type,params);
    sr.train(T,VI,VQ);
    sr.altSave(params["srOutput"],"");

    std::cout << "trained KSVD ok" << endl;

    return 0;
}


int testBucketCapacity(int argc, char *argv[]){

    string paramFile(argv[1]);

	map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

    arma::fmat T,D, Dt;
    D.load(parameters[argv[2]]);
    T.load(parameters[argv[3]]);

    Dt = D.t();

    cout << "D cols: " << D.n_cols << "\tD rows: " << D.n_rows << endl;

    int max_iters = 10;
    double eps = 1e-7;

    int max = std::numeric_limits<int>::min();
    int min = std::numeric_limits<int>::max();
    int total = 0;

    arma::uvec nonZeroCount = zeros<uvec>(D.n_cols);
    arma::fvec nonZeroSum = zeros<fvec>(D.n_cols);
    double totalRecError = 0;

    SRExtractor sr(D,max_iters,eps);
    for(uint i = 0; i < T.n_cols; i++){
        arma::fmat src = T.col(i);
        arma::fmat dst;
        sr.extractFeatures(src,dst);

        int count = 0;
        for(uint j = 0; j < dst.n_cols; j++){
            if(dst(0,j) > 0){
                count++;
                nonZeroCount.at(j)++;
                nonZeroSum.at(j)+= dst(0,j);
            }
        }
        if(count < min)
            min = count;
        if(count > max)
            max = count;

        total+= count;

        totalRecError+= arma::accu(arma::abs(src-(dst*Dt).t()));


    }
    for(uint j = 0; j < nonZeroCount.n_rows; j++){
            cout << nonZeroCount(j) << " \t";
    }
    cout << endl;

    for(uint j = 0; j < nonZeroSum.n_rows; j++){
            cout << nonZeroSum(j) << " \t";
    }
    cout << endl;

    cout << min << endl << max << endl << total/(int)nonZeroCount.n_rows << endl << totalRecError/T.n_cols << endl;

    return 0;
}

int testArmaWritePython(int argc, char *argv[]){
    arma::fmat T;
    T.load("/home/amourao/Desktop/a.mat");
    T.save("/home/amourao/Desktop/b.mat");
    cout << T << endl;
    return 0;
}

int testANdMP(int argc, char *argv[]){
    string paramFile(argv[1]);

	map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

    arma::fmat T,D, Dt;
    D.load(parameters[argv[2]]);
    T.load(parameters[argv[3]]);

    Dt = D.t();

    int max_iters = 10;
    double eps = 1e-7;

    SRExtractor sr(D,max_iters,eps);
    ANdOMPExtractor andMP(D,max_iters,eps);

    arma::fmat src = T.col(0);
    arma::fmat dst1,dst2;
    sr.extractFeatures(src,dst1);
    andMP.extractFeatures(src,dst2);

    cout << arma::accu(arma::abs(dst1-dst2)) << endl;

    return 0;
}

int main(int argc, char *argv[])
{
    srand (time(NULL));
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

    //FrameFilter::maine(argc, argv);
    //classifySapoAllVideos(argc, argv);
    //extractREST(argc, argv);
	//createBlipKnnVWDict(argc, argv);

    //extractAndSaveToBerkeleyDB(argc, argv);
	//readBerkeleyDB(argc, argv);
	cout << argv[1] << endl;

	if(StringTools::endsWith(string(argv[1]),"testBucketCapacity"))
        testBucketCapacity(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"trainKSVD"))
        trainKSVD(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"testANdMP"))
        testANdMP(argc-1,&argv[1]);

    return 0;
}

