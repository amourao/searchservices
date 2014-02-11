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
#include "tools/oneBillionImporter.h"
#include "tools/IBinImporter.h"


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

	std::pair< vector<float> , vector<float> > r = vw->knnSearchId(q,10);
	for(uint i = 0; i < r.first.size(); i++){
		cout << r.first.at(i)<< "\t" << r.second.at(i) << endl;
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


void testIndeces(int argc, char *argv[]){

	string file(argv[1]);

	map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;

	LoadConfig::load("config.json",parameters,indexers,analysers);


    IBinImporter* importer = new tinyImageImporter();


	int nTrain = atoi(parameters["nTrain"].c_str());
	int nValI = atoi(parameters["nValI"].c_str());
    int nValQ = atoi(parameters["nValQ"].c_str());
	int nTesI = atoi(parameters["nTesI"].c_str());
	int nTesQ = atoi(parameters["nTesQ"].c_str());
	int k = atoi(parameters["k"].c_str());

	Mat featuresTrain;
	Mat featuresValidationI;
	Mat featuresValidationQ;
	Mat featuresTestI;
	Mat featuresTestQ;

	//Mat labels;
    int currentOffset = 0;
	importer->readBin(file,nTrain,featuresTrain,currentOffset);
	currentOffset += nTrain;
	importer->readBin(file,nValI,featuresValidationI,currentOffset);
	currentOffset += nValI;
	importer->readBin(file,nValQ,featuresValidationQ,currentOffset);
	currentOffset += nValQ;
	importer->readBin(file,nTesI,featuresTestI,currentOffset);
	currentOffset += nTesI;
	importer->readBin(file,nTesQ,featuresTestQ,currentOffset);
	//importer->readBin(file,n*0.1,featuresValidation,n);
	//importer->readBin(file,n*0.1,featuresValidation,n);



	timestamp_type start, end;

	cout << "Training" << endl;
	for(int i = 0; i < indexers.size(); i++){
		get_timestamp(&start);

        indexers.at(i)->train(featuresTrain,featuresValidationQ,featuresValidationI);

		get_timestamp(&end);
		cout << indexers.at(i)->getName() << " " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
	}

	cout << "Indexing" << endl;
	for(int i = 0; i < indexers.size(); i++){
		get_timestamp(&start);

        indexers.at(i)->indexWithTrainedParams(featuresTestI);

		get_timestamp(&end);
		cout << indexers.at(i)->getName() << " " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
	}

	cout << endl << "Querying" << endl;
	for(int i = 0; i < indexers.size(); i++){

		Mat q = featuresTestQ.row(0);
		get_timestamp(&start);
		std::pair<vector<float>, vector<float> > r = indexers.at(i)->knnSearchId(q,k);
		get_timestamp(&end);
		cout << indexers.at(i)->getName() << " " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
		for(uint i = 0; i < r.first.size(); i++){
			cout << r.first.at(i) << "\t" << r.second.at(i) << endl;
		}
	}
}


void awesomeIndexTester(int argc, char *argv[]){

	map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;

    IBinImporter* importer;

    string paramFile(argv[1]);
	LoadConfig::load(paramFile,parameters,indexers,analysers);

    string file(parameters["file"]);
    string type(parameters["type"]);

    if(type == "tiny"){
        importer = new tinyImageImporter();
    } else if(type == "billion"){
        importer = new oneBillionImporter();
    } else {
        cout << "Unknown parameter value \"type\" = " << type << endl;
        return;
    }


	int nTrain = atoi(parameters["nTrain"].c_str());
	int nValI = atoi(parameters["nValI"].c_str());
    int nValQ = atoi(parameters["nValQ"].c_str());
	int nTesI = atoi(parameters["nTesI"].c_str());
	int nTesQ = atoi(parameters["nTesQ"].c_str());
	int k = atoi(parameters["k"].c_str());

    int tmp = 0;

	Mat featuresTrain;
	Mat featuresValidationI;
	Mat featuresValidationQ;
	Mat featuresTestI;
	Mat featuresTestQ;

    timestamp_type start, end;
    cout << "Reading featuresTrain: ";
	//Mat labels;
    int currentOffset = 0;
    get_timestamp(&start);
	importer->readBin(file,nTrain,featuresTrain,currentOffset);
	get_timestamp(&end);
	currentOffset += nTrain;
	cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationI ";
	get_timestamp(&start);
	importer->readBin(file,nValI,featuresValidationI,currentOffset);
	get_timestamp(&end);
	currentOffset += nValI;
    cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationQ ";
    get_timestamp(&start);
	importer->readBin(file,nValQ,featuresValidationQ,currentOffset);
	get_timestamp(&end);
	currentOffset += nValQ;
	cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestI ";
	/*
	get_timestamp(&start);
	importer->readBin(file,nTesI,featuresTestI,currentOffset);
	get_timestamp(&end);
	currentOffset += nTesI;
	cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ ";
	get_timestamp(&start);
	importer->readBin(file,nTesQ,featuresTestQ,currentOffset);
	get_timestamp(&end);
	cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
    */

    featuresValidationI.copyTo(featuresTestI);
    featuresValidationQ.copyTo(featuresTestQ);

	cout << "Training" << endl;
	for(int i = 0; i < indexers.size(); i++){
		get_timestamp(&start);

        indexers.at(i)->train(featuresTrain,featuresValidationQ,featuresValidationI);

		get_timestamp(&end);
		cout << indexers.at(i)->getName() << "\t" << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
	}

	cout << endl << "Indexing" << endl;
	for(int i = 0; i < indexers.size(); i++){
		get_timestamp(&start);

        indexers.at(i)->indexWithTrainedParams(featuresTestI);

		get_timestamp(&end);
		cout << indexers.at(i)->getName() << "\t" << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
	}

	cout << endl << "Querying" << endl;

	vector<std::pair<vector<float>, vector<float> > > linearResults;
	for(int i = 0; i < indexers.size(); i++){

        int tmp = 0;

        vector<float> precVsLinearTmp;
        double tmpTime = 0;
        IIndexer* ind = indexers.at(i);
        std::pair<vector<float>, vector<float> > r;
        vector<std::pair<vector<float>, vector<float> > > rAll;

        for (int j = 0; j < featuresTestQ.rows; j++){
            Mat q = featuresTestQ.row(j);
            get_timestamp(&start);
            r = ind->knnSearchId(q,k);
            get_timestamp(&end);
            tmpTime += timestamp_diff_in_milliseconds(start, end);
            if (i == 0)
                linearResults.push_back(r);
            rAll.push_back(r);
        }
        int correct = 0;
        int incorrect = 0;
        int kLinear = 0;

        for (int j = 0; j < rAll.size(); j++){
            int kLinear = 0;
            for (int m = 0; m < rAll.at(j).first.size(); m++){
                if (kLinear < rAll.at(j).first.size() &&

                (linearResults.at(j).first.at(kLinear) == rAll.at(j).first.at(m))

                ){
                    correct++;
                    kLinear++;
                } else {
                    incorrect++;
                    kLinear+=2;
                }
            }
        }
		cout << ind->getName() << "\t" << 100.0*correct/(incorrect+correct) << "%\t" <<  tmpTime <<  " ms" << endl;

	}
}

int main(int argc, char *argv[]){
	awesomeIndexTester(argc, argv);
    return 0;
}
