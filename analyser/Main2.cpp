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
#include "nVector/LBPExtractor.h"
#include "nVector/NullExtractor.h"
#include "nKeypoint/SURFExtractor.h"
#include "nVector/HistogramExtractor.h"
#include "nVector/SegmentedHistogramExtractor.h"

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

		int isd = 0;
		int isd2 = 50;
		
		cv::Mat gaborOwnNeutral = gaborOwnNeutralD[is.getCurrentImageInfoField(1)];
		cv::Mat lbpOwnNeutral = lbpOwnNeutralD[is.getCurrentImageInfoField(1)];
		
		gaborOwnNeutralD[is.getCurrentImageInfoField(1)];
		//float id;
		//float detected;
		float expected;

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

	
	int divisions =  atoi(argv[2]);

	vector<IClassifier*> classi; //choose the classifiers to test
	string dummy = "";
	string randf = StringTools::genRandom(5);
	stringstream sss;
	sss << "./tmpData/" << randf << "";
	randf = sss.str();
	classi.push_back(new SRClassifier(dummy));
	classi.push_back(new SVMClassifier(dummy));
	classi.push_back(new kNNClassifier());
	classi.push_back(new VWBasicClassifier(randf));

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


	int n = atoi(argv[2]);
	int w = atoi(argv[3]);
	int k = atoi(argv[4]);

	Mat features;
	//Mat labels;
	
	tinyImageImporter::readBin(file,n,features);
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
	IIndexer* ms = new MSIDXIndexer(dummy,w);

	linear->index(features);
	lsh->index(features);
	ms->index(features);

	lsh->save("L");
	lsh->load("L");
	Mat q = features.row(0);
	
	vector<std::pair<float,float> > r = linear->knnSearchId(q,k);
	for(uint i = 0; i < r.size(); i++){
		cout << r.at(i).first << "\t" << r.at(i).second << endl;
	}

	cout  << endl;
	cout  << endl;
	
	r = lsh->knnSearchId(q,k);
	for(uint i = 0; i < r.size(); i++){
		cout << r.at(i).first << "\t" << r.at(i).second << endl;
	}

	cout  << endl;
	cout  << endl;
	
	r = ms->knnSearchId(q,k);
	for(uint i = 0; i < r.size(); i++){
		cout << r.at(i).first << "\t" << r.at(i).second << endl;
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
}

int main(int argc, char *argv[])
{	
	//testLoadSaveIClassifier(argc, argv);
	//testLoadSaveIIndexer(argc, argv);
	//faceDetectionParameterChallenge(argc, argv);
    testAllClassifiersBin(argc, argv);
    //merger(argc, argv);
    //extractAllFeaturesCK(argc, argv);
	//testMSIDXIndexer(argc, argv);
    return 0;
}
