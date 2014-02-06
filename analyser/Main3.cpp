#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <opencv2/features2d/features2d.hpp>
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
#include "../indexer/LSHkNNIndexer.h"
#include "../indexer/LSHIndexer.h"

#include "../commons/StringTools.h"
#include "../commons/Timing.h"




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

void testMSIDXIndexer(int argc, char *argv[]){
	string file(argv[1]);

	int n = atoi(argv[2]);
	int k = atoi(argv[3]);

	Mat features;
	Mat featuresValidation;
	//Mat labels;

	tinyImageImporter::readBin(file,n,features);
	tinyImageImporter::readBin(file,n*0.1,featuresValidation,n);

	vector<IIndexer*> indexers;
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
	paramsL["multi_probe_level"] = "O";
	//table_number the number of hash tables to use [10...30]
    //key_size the size of the hash key in bits [10...20]
    //multi_probe_level the number of bits to shift to check for neighboring buckets
    //(0 is regular LSH, 2 is recommended).
	IIndexer* lsh = new LSHkNNIndexer(dummy,paramsL);

	map<string,string> paramsK;
	paramsK["algorithm"] = "kd";
	paramsK["distance"] = "EUCLIDEAN";
	paramsK["trees"] = "8";
	IIndexer* kd = new FlannkNNIndexer(dummy,paramsK);

	map<string,string> paramsKM;
	paramsKM["algorithm"] = "kmeans";
	paramsKM["distance"] = "EUCLIDEAN";
	paramsKM["branching"] = "32";
	paramsKM["iterations"] = "11";
	paramsKM["cb_index"] = "0.2";
	paramsKM["centers_init"] = "CENTERS_RANDOM";

	IIndexer* kmeans = new FlannkNNIndexer(dummy,paramsKM);

    map<string,string> paramsW;
	paramsW["w"] = "1";
	IIndexer* ms = new MSIDXIndexer(dummy,paramsW);


    map<string,string> paramsLSH;
	paramsLSH["oneMinusDelta"] = "0.99";
	paramsLSH["radius"] = "1.2";
	paramsLSH["trainValSplit"] = "0.95";
	IIndexer* e2lsh = new LSHIndexer(dummy,paramsLSH);


    indexers.push_back(linear);
    indexers.push_back(e2lsh);
	//indexers.push_back(lsh);
	indexers.push_back(kd);
	indexers.push_back(kmeans);
	indexers.push_back(ms);

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
	testMSIDXIndexer(argc, argv);
    return 0;
}
