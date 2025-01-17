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

#include "nVector/GaborExtractor.h"
#include "nKeypoint/SURFExtractor.h"
#include "nVector/HistogramExtractor.h"
#include "nVector/SegmentedHistogramExtractor.h"

#include "nRoi/FaceDetection.h"

#include "nTag/kNNClassifier.h"
#include "nTag/SVMClassifier.h"

#include "tools/TrainTestFeaturesTools.h"
#include "tools/MIRFlickrImporter.h"

#include "FactoryAnalyser.h"

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



void testDetection(int argc, char *argv[]) {

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

void extractAllFeaturesImEmotion(string testPath, string output) {

	TextFileSource is(testPath);

	//HistogramExtractor histogramExtractor (8);

	vector<cv::Rect> rectangleRois = vector<cv::Rect>();

	 rectangleRois.push_back(cv::Rect(0,0,46,64));
	 rectangleRois.push_back(cv::Rect(46,64,46,112-64));
	 rectangleRois.push_back(cv::Rect(46,0,46,64));
	 rectangleRois.push_back(cv::Rect(0,64,46,112-64));
	 rectangleRois.push_back(cv::Rect(0,10,92,30));
	 rectangleRois.push_back(cv::Rect(20,65,52,30));

	/*
	rectangleRois.push_back(cv::Rect(29, 13, 36, 20));
	rectangleRois.push_back(cv::Rect(10, 59, 24, 28));
	rectangleRois.push_back(cv::Rect(61, 57, 24, 29));
	rectangleRois.push_back(cv::Rect(55, 38, 17, 15));
	rectangleRois.push_back(cv::Rect(32, 28, 11, 16));
	rectangleRois.push_back(cv::Rect(10, 78, 26, 14));
	rectangleRois.push_back(cv::Rect(17, 61, 16, 13));
	rectangleRois.push_back(cv::Rect(64, 59, 15, 15));
	rectangleRois.push_back(cv::Rect(62, 73, 23, 18));

	rectangleRois.push_back(cv::Rect(13, 60, 18, 16));
	rectangleRois.push_back(cv::Rect(64, 55, 20, 20));
	rectangleRois.push_back(cv::Rect(23, 4, 25, 12));
	rectangleRois.push_back(cv::Rect(51, 5, 24, 11));
	rectangleRois.push_back(cv::Rect(34, 59, 13, 6));
	rectangleRois.push_back(cv::Rect(50, 58, 14, 8));
	rectangleRois.push_back(cv::Rect(7, 4, 27, 8));
	rectangleRois.push_back(cv::Rect(61, 4, 26, 8));
	rectangleRois.push_back(cv::Rect(25, 80, 17, 23));
	rectangleRois.push_back(cv::Rect(48, 78, 20, 25));
	*/
	GaborExtractor faceGaborExtractor(92, 112, 4, 6, rectangleRois);

	fstream faceGaborExtractorF(output.c_str(),
			std::ios::out | std::ios::binary);

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

	faceGaborExtractorF.write((const char*) &faceGaborExtractorS,
			sizeof(float));
	//faceEigenExtractorF.write( (const char*)& faceEigenExtractorS, sizeof(float) );
	//histogramExtractorF.write( (const char*)& histogramExtractorS, sizeof(float) );

	faceGaborExtractorF.write((const char*) &size, sizeof(float));
	//faceEigenExtractorF.write( (const char*)& size, sizeof(float) );
	//histogramExtractorF.write( (const char*)& size, sizeof(float) );

	int u = 0;

	double lastT = cvGetTickCount();

	while (!(src = is.nextImage()).empty()) {
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

		istringstream(expectedS) >> expected;

		if (!neutralPath.empty()) {

			cv::Mat neutral = imread(neutralPath);
			//faceEigenExtractor.extractFeatures(src,eigen);
			//histogramExtractor.extractFeatures(src,hist);
			faceGaborExtractor.extractFeatures(src, gabor);
			faceGaborExtractor.extractFeatures(neutral, gaborNeutral);

			cv::Mat gaborMinusNeutral = gabor - gaborNeutral;

			normalize(gaborMinusNeutral, gaborMinusNeutral, 0, 1, CV_MINMAX);

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
			faceGaborExtractorF.write((const char*) &expected, sizeof(float));

			for (int i = 0; i < gabor.cols; i++) {
				faceGaborExtractorF.write((const char*) &gabor.at<float>(0, i),
						sizeof(float));
			}

			u++;

			if (u % 250 == 0) {
				cout << u << endl;
				double rem = ((size - u) / (60 * 250))
						* ((double) cvGetTickCount() - lastT)
						/ ((double) cvGetTickFrequency() * 1000000);
				cout << rem << " minutes remaining" << endl;
				lastT = cvGetTickCount();
			}

		}

	}

	//faceEigenExtractorF.close();
	faceGaborExtractorF.close();
	//histogramExtractorF.close();

}

int testSURF(int argc, char *argv[]) {
	//extractAllFeaturesImEmotion(argv[1],argv[2]);

	KeypointFeatureExtractor* s = new SURFExtractor(1000);

	string file(argv[1]);

	vector<pair<vector<float>, vector<float> > > keypoints;

	s->extractFeatures(file, keypoints);

	cout << keypoints.at(0).first.size() << " " << keypoints.at(0).first.size()
			<< endl;
	cout << keypoints.at(0).second.size() << " " << keypoints.at(0).second.at(0)
			<< " " << keypoints.at(0).second.at(1) << endl;

	return 0;
}

int extractHistogram(int argc, char *argv[]){

	int tmpError = 0;

	string imageBasePath(argv[4]);

	string file(argv[1]);

	TextFileSource is(file,imageBasePath);

	FeatureExtractor* s = new HistogramExtractor(atoi(argv[3]));


	fstream binFine(argv[2], std::ios::out | std::ios::binary);
	//FeatureExtractor* s = new NullExtractor();

	float featureVectorCount = is.getImageCount();
	float dimensionality = s->getFeatureVectorSize();

	binFine.write((const char*) &dimensionality,sizeof(float));
	binFine.write((const char*) &featureVectorCount,sizeof(float));

	cv::Mat src;
	cv::Mat dst;

	int i = 0;
	double t = (double)cvGetTickCount();
	cout << "getImageCount: " << is.getImageCount() << endl;
	for (int k = 0; k < is.getImageCount(); k++) {
		if (!(src = is.nextImage()).empty()) {

			//cout << "ok" << endl;

			cv::Mat featuresRow;
			if (src.channels() == 1) {
				cv::cvtColor(src, dst, CV_GRAY2BGR);
			} else
				dst = src;

			s->extractFeatures(dst, featuresRow);

			string path, idStr, tmp1, tmp2;

			stringstream liness(is.getImageInfo());

			getline(liness, idStr, '\t');
			getline(liness, path);


			float label = atoi(idStr.c_str());
			if ((i - 1) % 50 == 0){
				cout << "image " << i << " of " << is.getImageCount() << endl;

				double time = (double)cvGetTickCount()-t;
				t = cvGetTickCount();
				int remaining = is.getImageCount() - i;
				printf( "time remaining = %g h\n", remaining * time/((double)cvGetTickFrequency()*1000000.* 3600. * 50.));
			}
			i++;

			binFine.write((const char*) &label,sizeof(float));

			for (int a = 0; a < dimensionality; a++){
				float g = featuresRow.at<float>(0,a);
				binFine.write((const char*) &g,sizeof(float));
			}


		}
	}
	binFine.close();

	delete s;
	return 0;
}

int testEverything(int argc, char *argv[]) {

// /home/amourao/iclefmed12/fast.hevs.ch/imageclefmed/2012/modality-classfication/tmp.txt
//"ImageCLEF 2012 Training Set/DVDM/IJD-54-150-g002.jpg";IJD-54-150-g002;DVDM;16

	int tmpError = 0;

	string file(argv[1]);


	TextFileSource is(file);


	FeatureExtractor* s = new SegmentedHistogramExtractor(atoi(argv[3]),atoi(argv[4]), atoi(argv[5]),false);
	//FeatureExtractor* s = new HistogramExtractor(atoi(argv[3]));

	//FeatureExtractor* s = new NullExtractor();

	cv::Mat src;
	cv::Mat dst;
	cv::Mat features;
	cv::Mat labels;
	int i = 0;

	cout << is.getImageCount() << endl;
	for (int k = 0; k < is.getImageCount(); k++) {
		if (!(src = is.nextImage()).empty()) {
			cv::Mat featuresRow;
			if (src.channels() == 1) {
				cv::cvtColor(src, dst, CV_GRAY2BGR);
			} else
				dst = src;
			s->extractFeatures(dst, featuresRow);

			cv::Mat label(1, 1, CV_32F);

			string path, idStr, tmp1, tmp2;

			stringstream liness(is.getImageInfo());

			getline(liness, path, ';');
			getline(liness, tmp1, ';');
			getline(liness, tmp2, ';');
			getline(liness, idStr, '\r');

			label.at<float>(0, 0) = atoi(idStr.c_str());
			if ((i - 1) % 50 == 0)
				cout << "a" << i << " " << is.getImageCount() << " " << path
						<< endl;
			i++;
			features.push_back(featuresRow);
			labels.push_back(label);
	//cout <<	tmpError++ << endl;
		}
	}
	//cv::Mat featuresF;

	features.convertTo(features,CV_32F);
	labels.convertTo(labels,CV_32F);

	IClassifier* sr = new SRClassifier();
	IClassifier* knn = new kNNClassifier();
	IClassifier* svm = new SVMClassifier();

	vector<IClassifier*> classi;
	classi.push_back(sr);
	classi.push_back(knn);
	classi.push_back(svm);

	TrainTestFeaturesTools ttft(features, labels, classi);

	cout << ttft.crossValidateAll(atoi(argv[2])) << endl;

	return 0;
}

int testEverythingBin(int argc, char *argv[]) {

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
	//classi.push_back(new kNNClassifier());
	classi.push_back(new SVMClassifier());

	//cout << features.rows << " " << features.cols << " " << labels.rows << " " << labels.cols << endl;
	TrainTestFeaturesTools ttft(features, labels, testFeatures, testLabels,
			classi);

	//cout << ttft.crossValidateAll(atoi(argv[2])) << endl;
	//ttft.splitDataForTest(0.3);
	cout << ttft.testAll() << endl;

	return 0;
}

void readFile(string file, Mat& dataR, Mat& labelsR, FeatureExtractor* s) {
	cv::Mat features;
	cv::Mat labels;
	TextFileSource is(file);
	cv::Mat src;
	cv::Mat dst;
	//int i = 0;
	for (int k = 0; k < is.getImageCount(); k++) {
		if (!(src = is.nextImage()).empty()) {

			cv::Mat featuresRow;
			if (src.channels() == 1) {
				cv::cvtColor(src, dst, CV_GRAY2BGR);
			} else
				dst = src;
			s->extractFeatures(dst, featuresRow);

			cv::Mat label(1, 1, CV_32F);

			string path, idStr;

			stringstream liness(is.getImageInfo());

			getline(liness, path, ';');
			getline(liness, idStr, '\r');

			label.at<float>(0, 0) = atoi(idStr.c_str());
			//if((i+1) % 50 == 0)
			//	cout << "a" << i << " " << is.getImageCount() << " " << path <<  endl;
			//i++;
			features.push_back(featuresRow);
			labels.push_back(label);
		} else {
			string path, idStr;

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

int testINRIA(int argc, char *argv[]) {

	string trainfile(argv[1]);
	string testfile(argv[2]);

	vector<FeatureExtractor*> featureExtractors;
	featureExtractors.push_back(
			new SegmentedHistogramExtractor(atoi(argv[4]), atoi(argv[5]),
					atoi(argv[6])),false);
	featureExtractors.push_back(new HistogramExtractor(atoi(argv[4])));

	for (unsigned int i = 0; i < featureExtractors.size(); i++) {
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

		readFile(trainfile, features, labels, featureExtractors.at(i));
		readFile(testfile, featuresTest, labelsTest, featureExtractors.at(i));

		vector<IClassifier*> classi;
		classi.push_back(new SRClassifier());
		classi.push_back(new kNNClassifier());
		//classi.push_back(new SVMClassifier());

		TrainTestFeaturesTools ttft(features, labels, featuresTest, labelsTest,
				classi);

		cout << ttft.testAll() << endl;
	}
	return 0;
}

int testInterfaces(int argc, char *argv[]) {

	string trainfile(argv[1]);

	FeatureExtractor* featureExtractor = new SegmentedHistogramExtractor(
			atoi(argv[2]), atoi(argv[3]), atoi(argv[4]),false);
	FeatureExtractor* featureExtractor2 = new HistogramExtractor(atoi(argv[2]));

	vector<float> a;
	vector<float> b;

	featureExtractor->extractFeatures(trainfile, a);
	featureExtractor2->extractFeatures(trainfile, b);

	cout << a.size() << endl;
	cout << b.size() << endl;

	cout << a.at(0) << endl;
	cout << b.at(0) << endl;

	getchar();
	return 0;
}

int testFactories(int argc, char *argv[]) {

	FactoryAnalyser * f = FactoryAnalyser::getInstance();

	f->listTypes();

	cout << "bla2" << endl;
	//cout <<  f->createType(featureExtractor->getName()) << endl;

	return 0;
}

int generateRandomData(char* filename) {

	fstream binFile;
	binFile.open(filename, std::ios::out | std::ios::binary);

	float dims = 10000;
	binFile.write((const char*) &dims, sizeof(float));
	float nSamples = 512;
	binFile.write((const char*) &nSamples, sizeof(float));

	for (int s = 0; s < nSamples; s++) {

		float e = (int) floor(nSamples / 8);
		binFile.write((const char*) &e, sizeof(float));
		binFile.write((const char*) &e, sizeof(float));

		for (int i = 0; i < 10000; i++) {
			float value = 16 * e + rand() % 8;
			binFile.write((const char*) &value, sizeof(float));
		}
		binFile.flush();
	}

	binFile.close();

}



int evaluateMIRFlickr(int argc, char *argv[]) {


	std::ofstream out("out.txt");
	std::streambuf *coutbuf = std::cout.rdbuf(); //save old buf
	std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!


	MIRFlickrImporter m;

	//1: alltag list fils


	string tagsFile(argv[1]);
	string filename(argv[2]);
	int testSize = atoi(argv[3]);


	arma::fmat features;
	arma::fmat trainFeatures;
	arma::fmat testFeatures;

	std::map<std::string,arma::uvec> trainTags;
	std::map<std::string,arma::uvec> testTags;

	m.readBin(filename, features);
	m.readAllTags(tagsFile);

	m.getTrainData(trainFeatures,trainTags);
	m.getTestData(testFeatures,testTags);


	arma::fmat tags = arma::zeros<arma::fmat>((int)trainFeatures.n_rows,1);
	SRClassifier sr;

	sr.train(trainFeatures,tags);

	vector<priority_queue<tagError, vector<tagError>, CompareTags> > resultingTags(testFeatures.n_rows);

	std::map<std::string, arma::uvec>::iterator iter;
	for (iter = trainTags.begin(); iter != trainTags.end(); ++iter) {
		//cout << iter->first << endl;
		arma::uvec indexes = iter->second;

		tags = arma::zeros<arma::fmat>((int)trainFeatures.n_rows,1);
		for (int i = 0; i < indexes.n_rows; i++){
			int index = indexes(i);
			tags.at(index,0) = 1;
			//indexes.at(m.trainDataIndex.at(index),0) = 1;
		}
		//cout << trainFeatures.n_rows << " " << trainFeatures.n_cols << endl;

		/*
		arma::uvec filteredCols(features.n_cols);
		for(unsigned int s = 0; s < features.n_cols; s++)
			filteredCols(s) = s;
		arma::fmat featuresTags = features.submat(indexes,filteredCols);
		arma::fmat tmopTags = arma::ones<arma::fmat>(featuresTags.n_rows,1);
		sr.train(featuresTags,tmopTags);
		 */

		sr.changeLabels(tags);

		for (int row = 0; row < testSize; row++) {
			double * error = new double();
			arma::fmat* featuresTmp = new arma::fmat();
			sr.classify(testFeatures.row(row),error,featuresTmp);
			//cout << m.testDataIndex.at(row) << " " <<  (*error) << endl;

			resultingTags.at(m.testDataIndex.at(row)).push({(*error),iter->first});

		}
	}

	for (int row = 0; row < testSize; row++) {
		cout << "detected tags:\t" << m.testDataIndex.at(row)+1 << endl;
		priority_queue<tagError, vector<tagError>, CompareTags> tagsQueue = resultingTags.at(m.testDataIndex.at(row));
		while (tagsQueue.size() > 0){
			tagError terr = tagsQueue.top();
			tagsQueue.pop();
			bool contains = false;

			vector<string> vec = m.testTags.at( m.testDataIndex.at(row));
			if(std::find(vec.begin(), vec.end(), terr.tag) != vec.end())
				contains = true;
			cout << std::setprecision(5);
			if(terr.tag.size() > 7)
				cout << terr.tag << "\t" << terr.error << "\t\t" << contains << endl;
			else
				cout << terr.tag << "\t\t" << terr.error << "\t\t" << contains << endl;
		}
		cout << endl;
	}

	//m.getTagFeatures("people_r1", people_r1Tags);
	//m.getTagFeatures("female_r1", female_r1Tags);
	//m.subtractTags("people_r1","female_r1",subtractTags);
	//cout << people_r1Tags.n_rows << " " << female_r1Tags.n_rows << " " << subtractTags.n_rows << endl;



	//m.getTag("baby_r1",tags);
	//arma::uvec subtractTags;
	//m.subtractTags("baby.txt","baby_r1.txt",subtractTags);

	//the tags must be ordered
	/*
	 int lastIdOfTheTaggedElement = 0;
	 int tmpIndex = 0;
	 for(unsigned int s = 1; s <= features.n_rows; s++){
	 if(lastIdOfTheTaggedElement >= tags.n_rows || s != tags(lastIdOfTheTaggedElement)) {
	 withoutTags.insert_rows(tmpIndex,1);
	 withoutTags.at(tmpIndex++) = s;
	 } else {
	 lastIdOfTheTaggedElement++;
	 }
	 }
	 */

	//arma::uvec filteredCols(features.n_cols);
	//for(unsigned int s = 0; s < features.n_cols; s++)
	//	filteredCols(s) = s;

	//arma::uvec filteredColsU = arma::conv_to<arma::uvec>::from(filteredCols);
	//arma::fmat featuresTags = features.submat(tags,filteredCols);
	//arma::fmat featuresWithoutTags = features.submat(withoutTags,filteredCols);
	//arma::fmat featuresWithoutTags = features.submat(testTags,filteredCols);
	//SRClassifier sr;
	//arma::fmat tagsF = arma::conv_to<arma::fmat>::from(tags);
	//arma::fmat tagsF = arma::ones<arma::fmat>(tags.n_rows,1);
	//sr.train(featuresTags.rows(0,10),tagsF.rows(0,10));

	//arma::fmat* b = new arma::fmat();

	//cout << featuresTags.row(0) << endl;
	/*
	 double average = 0;
	 double count = 0;

	 for (int i = 0; i < 10; i++){
	 double* a = new double();
	 if (sr.classify(featuresTags.row(10+1+i),a,b) != -1){
	 average += (*a);
	 count++;
	 }
	 delete a;
	 }

	 cout << average/count << endl;

	 average = 0;
	 count = 0;
	 for (int i = 0; i < 10; i++){
	 double* a = new double();
	 if (sr.classify(featuresWithoutTags.row(i),a,b)!= -1){
	 average += (*a);
	 count++;
	 }
	 delete a;
	 }

	 cout << average/count << endl;
	 */

	return 0;
}













int mainAnalyser(int argc, char *argv[]) {



	extractHistogram(argc, argv);
	//testEverything(argc, argv);
	//evaluateMIRFlickr(argc, argv);
	//extractAllFeaturesImEmotion(
	//testEverythingBin(argc,argv);

	//generateRandomData(argv[1]);
	//getchar();
	return 0;
}

