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
#include "tools/FrameFilter.h"

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
	classi.push_back(new SRClassifier(dummy));
	classi.push_back(new SVMClassifier(dummy));
	classi.push_back(new kNNClassifier());
    map<string,string> params;
    params["trainFile"] = randf;

	//classi.push_back(new VWBasicClassifier(dummy,params));

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




int main(int argc, char *argv[])
{
    return testAllClassifiersBin(argc, argv);
}

