#include <iostream>
#include <string>
#include <sstream>
#include <set>
#include <cstdlib>
#include <opencv2/features2d/features2d.hpp>
#include <time.h>
//#include <math>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>

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
#include "../indexer/LSHIndexer.h"

#include "../commons/StringTools.h"
#include "../commons/Timing.h"
#include "../commons/LoadConfig.h"



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

void testIndeces(int argc, char *argv[]){
	string file(argv[1]);

	int n = atoi(argv[2]);
	int nTrain = atoi(argv[2]);
	int nVal = atoi(argv[2]);
	int k = atoi(argv[3]);

	Mat featuresTrain;
	Mat featuresValidationQ;
	Mat featuresValidationI;
	Mat featuresTestQ;
	Mat featuresTestI;

    Mat features;
	Mat featuresValidation;
	//Mat labels;

	tinyImageImporter::readBin(file,n,featuresTrain);
	//tinyImageImporter::readBin(file,n*0.1,featuresValidation,n);
	//tinyImageImporter::readBin(file,n*0.1,featuresValidation,n);

	vector<IIndexer*> indexers = LoadConfig::getRegisteredIndeces("config.json");

	timestamp_type start, end;

	cout << "Indexing" << endl;
	for(int i = 0; i < indexers.size(); i++){
		get_timestamp(&start);

		if (indexers.at(i)->getName() == "E2LSHIndexer")
            ((LSHIndexer*)indexers.at(i))->index(features,featuresValidation);
		else
            indexers.at(i)->index(features);

		get_timestamp(&end);
		cout << indexers.at(i)->getName() << " " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
	}

	cout << endl << "Querying" << endl;
	for(int i = 0; i < indexers.size(); i++){

		Mat q = features.row(0);
		get_timestamp(&start);
		vector<std::pair<float,float> > r = indexers.at(i)->knnSearchId(q,k);
		get_timestamp(&end);
		cout << indexers.at(i)->getName() << " " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
		for(uint i = 0; i < r.size(); i++){
			cout << r.at(i).first << "\t" << r.at(i).second << endl;
		}
	}
}

int main(int argc, char *argv[]){
	testIndeces(argc, argv);
    return 0;
}
