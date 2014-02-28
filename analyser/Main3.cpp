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

#include "../indexer/sphericalHashing/Main.cpp"

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
    } else if(type == "nsBin"){
        importer = new MatrixTools();
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
	int mapAt = atoi(parameters["mapAt"].c_str());

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
    nTesI = featuresTestI.rows;
    nTesQ = featuresTestQ.rows;
    vector<std::pair<vector<float>, vector<float> > > linearResults;

    cout << endl << "nTrain;nValI;nValQ;nTesI;nTesQ;mAcc;k;d" << endl;
    cout << nTrain << ";" << nValI << ";" << nValQ << ";" << nTesI << ";" << nTesQ << ";" << k << ";" << featuresTestI.cols << endl << endl;

	cout << "Name;Name2;TrainTime;IndexingTime;QueryTime;%Correct;avgDeltaDistance" << endl;
	for(int i = 0; i < indexers.size(); i++){
		get_timestamp(&start);
        indexers.at(i)->train(featuresTrain,featuresValidationQ,featuresValidationI);
		get_timestamp(&end);

		int pos = indexers.at(i)->getName().rfind("_");
		string simpleName = indexers.at(i)->getName().substr(0,pos);

        cout << indexers.at(i)->getName() << ";" << simpleName << ";" << timestamp_diff_in_milliseconds(start, end);
		

		get_timestamp(&start);
        indexers.at(i)->indexWithTrainedParams(featuresTestI);
		get_timestamp(&end);

		cout << ";" << timestamp_diff_in_milliseconds(start, end);

        int tmp = 0;

        vector<float> precVsLinearTmp;
        double tmpTime = 0;
        std::pair<vector<float>, vector<float> > r;
        vector<std::pair<vector<float>, vector<float> > > rAll;

        for (int j = 0; j < featuresTestQ.rows; j++){
            Mat q = featuresTestQ.row(j);
            get_timestamp(&start);
            r =  indexers.at(i)->knnSearchId(q,k);
            get_timestamp(&end);
            tmpTime += timestamp_diff_in_milliseconds(start, end);
            if (i == 0)
                linearResults.push_back(r);
            rAll.push_back(r);
        }
        double deltaDistance = 0;

        long commonElements = 0;


        int relevant = 0;
        double avgPrec = 0;
        for (int j = 0; j < rAll.size(); j++){
            int kLinear = 0;
            long relAccum = 0;
            double precAccum = 0;

            for (int m = 0; m < rAll.at(j).first.size(); m++){
                deltaDistance += rAll.at(j).second.at(m) - linearResults.at(j).second.at(m);
            }

            for (int m = 0; m < rAll.at(j).first.size(); m++){

                for (int n = 0; n < mapAt; n++){
                    if (rAll.at(j).first.at(m) == linearResults.at(j).first.at(n)){
                        commonElements++;
                        relAccum++;
                        relevant = 1;
                    }
                }
                double precisionAtM = relAccum/(m+1.0);
                precAccum += precisionAtM*relevant;
            }
            avgPrec += precAccum;
        }
		cout << ";" <<  tmpTime <<  ";" << ((double)commonElements)/(k*nTesQ) << ";" << avgPrec/(k*nTesQ) << ";" << deltaDistance << endl;

		indexers.at(i)->saveParams(indexers.at(i)->getName());

		delete indexers.at(i);
	}
}


void computeGT(int argc, char *argv[]){

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
    } else if(type == "nsBin"){
        importer = new MatrixTools();
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
	//Mat labels;
    long currentOffset = 0;

	currentOffset = nTrain + nValI + nValQ;
	cout << "Reading featuresTestI " << endl;
	get_timestamp(&start);
	importer->readBin(file,nTesI,featuresTestI,currentOffset);
	get_timestamp(&end);
	currentOffset += nTesI;
	cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ " << endl;
	get_timestamp(&start);
	importer->readBin(file,nTesQ,featuresTestQ,currentOffset);
	get_timestamp(&end);
	cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;


    vector<std::pair<vector<float>, vector<float> > > linearResults;

    cout << endl << "file;nTrain;nValI;nValQ;nTesI;nTesQ;k;d" << endl;
    cout << file << ";" << nTrain << ";" << nValI << ";" << nValQ << ";" << nTesI << ";" << nTesQ << ";" << k << ";" << featuresTestI.cols << endl << endl;

    cout << endl;
    cout << nTesQ << endl;
    cout << k << endl;

	for(int i = 0; i < 1; i++){
		get_timestamp(&start);
        indexers.at(i)->train(featuresTrain,featuresValidationQ,featuresValidationI);
		get_timestamp(&end);

		get_timestamp(&start);
        indexers.at(i)->indexWithTrainedParams(featuresTestI);
		get_timestamp(&end);

        int tmp = 0;

        vector<float> precVsLinearTmp;
        double tmpTime = 0;
        std::pair<vector<float>, vector<float> > r;
        vector<std::pair<vector<float>, vector<float> > > rAll;

        for (int j = 0; j < featuresTestQ.rows; j++){
            Mat q = featuresTestQ.row(j);
            get_timestamp(&start);
            r =  indexers.at(i)->knnSearchId(q,k);
            get_timestamp(&end);
            tmpTime += timestamp_diff_in_milliseconds(start, end);
            for (int l = 0; l < r.first.size(); l++){
                cout << r.first.at(l) << "," << r.second.at(l) << ";";
            }
            cout << endl;
        }

		delete indexers.at(i);
	}
}

void loadGT(string& gtFile,vector<std::pair<vector<float>, vector<float> > >& linearResults,string &nameO){
    ifstream file(gtFile.c_str(), ifstream::in);
    string line, path, tmpStr;

    getline(file, line);
    string name = line;

    getline(file, line);
    int offset = atoi(line.c_str());

    getline(file, line);
    int nRows = atoi(line.c_str());

    getline(file, line);
    int nCols = atoi(line.c_str());

    for(int i = 0; i < nRows; i++){
        getline(file, line);
        stringstream liness(line);

        vector<float> indeces;
        vector<float> dists;
        for(int j = 0; j < nCols; j++){
            getline(liness, tmpStr, ',');
            indeces.push_back(atof(tmpStr.c_str()));
            getline(liness, tmpStr, ';');
            dists.push_back(atof(tmpStr.c_str()));
        }
        linearResults.push_back(make_pair(indeces,dists));
    }
}

void awesomeIndexTesterSingle(int argc, char *argv[]){

	map<string,string> parameters;
	vector<IIndexer*> allIndexers;
	vector<IAnalyser*> analysers;

	bool debug = false;

    IBinImporter* importer;

    string paramFile(argv[1]);

    int indexToTest = atoi(argv[2]);

	LoadConfig::load(paramFile,parameters,allIndexers,analysers);

	//cout << allIndexers.size() << endl;

    string file(parameters["file"]);
    string type(parameters["type"]);
    string gtFile(parameters["gtFile"]);

    if(type == "tiny"){
        importer = new tinyImageImporter();
    } else if(type == "billion"){
        importer = new oneBillionImporter();
    } else if(type == "nsBin"){
        importer = new MatrixTools();
    } else {
        cout << "Unknown parameter value \"type\" = " << type << endl;
        return;
    }

    int currentOffset = 0;

    if (parameters.count("debug") > 0){
	    debug = true;
	}

    if (parameters.count("startOffset") > 0){
		currentOffset = atoi(parameters["startOffset"].c_str());
	}

	int nTrain = atoi(parameters["nTrain"].c_str());
	int nValI = atoi(parameters["nValI"].c_str());
    int nValQ = atoi(parameters["nValQ"].c_str());
	int nTesI = atoi(parameters["nTesI"].c_str());
	int nTesQ = atoi(parameters["nTesQ"].c_str());
	int k = atoi(parameters["k"].c_str());
	int mapAt = atoi(parameters["mapAt"].c_str());



    int tmp = 0;

	Mat featuresTrain;
	Mat featuresValidationI;
	Mat featuresValidationQ;
	Mat featuresTestI;
	Mat featuresTestQ;

    timestamp_type start, end;
    if (debug) cout << "Reading featuresTrain: ";
	//Mat labels;
    
    get_timestamp(&start);
	importer->readBin(file,nTrain,featuresTrain,currentOffset);
	get_timestamp(&end);
	currentOffset += nTrain;
	if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationI ";
	get_timestamp(&start);
	importer->readBin(file,nValI,featuresValidationI,currentOffset);
	get_timestamp(&end);
	currentOffset += nValI;
    if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationQ ";
    get_timestamp(&start);
	importer->readBin(file,nValQ,featuresValidationQ,currentOffset);
	get_timestamp(&end);
	currentOffset += nValQ;
	if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestI ";
	get_timestamp(&start);
	importer->readBin(file,nTesI,featuresTestI,currentOffset);
	get_timestamp(&end);
	currentOffset += nTesI;
	if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ ";
	get_timestamp(&start);
	importer->readBin(file,nTesQ,featuresTestQ,currentOffset);
	get_timestamp(&end);
	if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;

    //featuresValidationI.copyTo(featuresTestI);
    //featuresValidationQ.copyTo(featuresTestQ);
    //nTesI = featuresTestI.rows;
    //nTesQ = featuresTestQ.rows;
    vector<std::pair<vector<float>, vector<float> > > linearResults;

	int posC = file.rfind("/");
	string simpleFileName = file.substr(0,posC);

	if (debug) cout << "Loading GT" << endl;
    loadGT(gtFile,linearResults,simpleFileName);
	if (debug) cout << "Loading GT ok" << endl;

    vector<IIndexer*> indexers;

    indexers.push_back(allIndexers.at(indexToTest));

    
    for(int i = 0; i < indexers.size(); i++){
    	
    	if (debug) cout << "Training" << endl;
		get_timestamp(&start);
        indexers.at(i)->train(featuresTrain,featuresValidationQ,featuresValidationI);
		get_timestamp(&end);
		if (debug) cout << "Training ok" << endl;

		int pos = indexers.at(i)->getName().rfind("_");
		string simpleName = indexers.at(i)->getName().substr(0,pos);

        cout << indexers.at(i)->getName() << ";" << simpleName << ";" << timestamp_diff_in_milliseconds(start, end);

        if (debug) cout << "Indexing" << endl;
		get_timestamp(&start);
        indexers.at(i)->indexWithTrainedParams(featuresTestI);
		get_timestamp(&end);
		if (debug) cout << "Indexing ok" << endl;

        //if(indexToTest == 0 || i > 0)
        cout << ";" << timestamp_diff_in_milliseconds(start, end);

        int tmp = 0;

        vector<float> precVsLinearTmp;
        double tmpTime = 0;
        std::pair<vector<float>, vector<float> > r;
        vector<std::pair<vector<float>, vector<float> > > rAll;

		if (debug) cout << "Querying" << endl;        
        for (int j = 0; j < featuresTestQ.rows; j++){
            Mat q = featuresTestQ.row(j);
            get_timestamp(&start);
            r =  indexers.at(i)->knnSearchId(q,k);
            get_timestamp(&end);
            tmpTime += timestamp_diff_in_milliseconds(start, end);
            rAll.push_back(r);
        }
        if (debug) cout << "Querying ok" << endl;
        double deltaDistance = 0;

        long commonElements = 0;


        int relevant = 0;
        double avgPrec = 0;
        for (int j = 0; j < rAll.size(); j++){
            int kLinear = 0;
            long relAccum = 0;
            double precAccum = 0;

            for (int m = 0; m < rAll.at(j).first.size(); m++){
                deltaDistance += rAll.at(j).second.at(m) - linearResults.at(j).second.at(m);
            }

            for (int m = 0; m < rAll.at(j).first.size(); m++){

                for (int n = 0; n < mapAt; n++){
                    if (rAll.at(j).first.at(m) == linearResults.at(j).first.at(n)){
                        commonElements++;
                        relAccum++;
                        relevant = 1;
                    }
                }
                double precisionAtM = relAccum/(m+1.0);
                precAccum += precisionAtM*relevant;
            }
            avgPrec += precAccum;
        }

        //if(indexToTest == 0 || i > 0)
        cout << ";" <<  tmpTime <<  ";" << ((double)commonElements)/(k*nTesQ) << ";" << avgPrec/(k*nTesQ) << ";" << deltaDistance << endl;

        stringstream ss2;

        int pos2a = paramFile.rfind("/");
        int pos2b = paramFile.rfind(".");

        if (pos2a == std::string::npos)
        	pos2a = 0;

		string simpleParamFileName = paramFile.substr(pos2a,pos2b-pos2a);

        ss2 << simpleParamFileName << "_" << std::setw(5) << std::setfill('0') << indexToTest << "_" << indexers.at(i)->getName();

		indexers.at(i)->saveParams(ss2.str());

		delete indexers.at(i);
	}
}

void testSphericalHashing(int argc, char *argv[]){

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
    } else if(type == "nsBin"){
        importer = new MatrixTools();
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

    featuresValidationI.copyTo(featuresTestI);
    featuresValidationQ.copyTo(featuresTestQ);

    float** d1 = new float* [featuresTestI.rows];
    float** d2 = new float* [featuresTestQ.rows];

    for(int i=0;i<featuresTestI.rows;i++){
        d1[i] = new float [ featuresTestI.cols ];
    }

    for(int i=0;i<featuresTestQ.rows;i++){
        d2[i] = new float [ featuresTestQ.cols ];
    }

    for (int i = 0; i < featuresTestI.rows; i++)
        for (int j = 0; j < featuresTestI.cols; j++)
            d1[i][j] = (float)( featuresTestI.at<float>(i,j) );

    for (int i = 0; i < featuresTestQ.rows; i++)
        for (int j = 0; j < featuresTestQ.cols; j++)
            d2[i][j] = (float)( featuresTestQ.at<float>(i,j) );

    //mainSphericalHashing().main(d1,d2,featuresTestI.rows,featuresTestQ.rows,featuresTestQ.cols);
}

int main(int argc, char *argv[]){
	//awesomeIndexTester(argc, argv);
	awesomeIndexTesterSingle(argc, argv);
	//testSphericalHashing(argc, argv);
	//computeGT(argc, argv);
    return 0;
}
