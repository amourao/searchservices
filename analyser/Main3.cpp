#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>
#include <cstdlib>
#include <opencv2/features2d/features2d.hpp>
#include <time.h>
#include <armadillo>

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
//#include "../indexer/SRIndexer.h"


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

	cv::Mat features;
	cv::Mat labels;

	MatrixTools::readBin(file, features, labels);
	IIndexer* vw = new FlannkNNIndexer();

	vw->index(features);
	vw->save("medicalImage_CEDD_kNN");
	cv::Mat q = features.row(0);

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
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load("config.json",parameters,indexers,analysers,classifiers,endpoints);


    IBinImporter* importer = new tinyImageImporter();


	int nTrain = atoi(parameters["nTrain"].c_str());
	int nValI = atoi(parameters["nValI"].c_str());
    int nValQ = atoi(parameters["nValQ"].c_str());
	int nTesI = atoi(parameters["nTesI"].c_str());
	int nTesQ = atoi(parameters["nTesQ"].c_str());
	int k = atoi(parameters["k"].c_str());

	cv::Mat featuresTrain;
	cv::Mat featuresValidationI;
	cv::Mat featuresValidationQ;
	cv::Mat featuresTestI;
	cv::Mat featuresTestQ;

	//cv::Mat labels;
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
	for(uint i = 0; i < indexers.size(); i++){
		get_timestamp(&start);

        indexers.at(i)->train(featuresTrain,featuresValidationQ,featuresValidationI);

		get_timestamp(&end);
		cout << indexers.at(i)->getName() << " " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
	}

	cout << "Indexing" << endl;
	for(uint i = 0; i < indexers.size(); i++){
		get_timestamp(&start);

        indexers.at(i)->indexWithTrainedParams(featuresTestI);

		get_timestamp(&end);
		cout << indexers.at(i)->getName() << " " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
	}

	cout << endl << "Querying" << endl;
	for(uint i = 0; i < indexers.size(); i++){

		cv::Mat q = featuresTestQ.row(0);
		get_timestamp(&start);
		std::pair<vector<float>, vector<float> > r = indexers.at(i)->knnSearchId(q,k);
		get_timestamp(&end);
		cout << indexers.at(i)->getName() << " " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
		for(uint j = 0; i < r.first.size(); i++){
			cout << r.first.at(j) << "\t" << r.second.at(j) << endl;
		}
	}
}

void awesomeIndexTesterOldAndWrong(int argc, char *argv[]){

    IBinImporter* importer;

    string paramFile(argv[1]);

    map<string,string> parameters;
    vector<IIndexer*> indexers;
    vector<IAnalyser*> analysers;
    vector<IClassifier*> classifiers;
    vector<IEndpoint*> endpoints;

    LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);
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
    string file,outfileNameA;

    int nTrain = atoi(parameters["nTrain"].c_str());
    int nValI = atoi(parameters["nValI"].c_str());
    int nValQ = atoi(parameters["nValQ"].c_str());
    int nTesI = atoi(parameters["nTesI"].c_str());
    int nTesQ = atoi(parameters["nTesQ"].c_str());
    int k = atoi(parameters["k"].c_str());
    int mapAt = atoi(parameters["mapAt"].c_str());

    //int tmp = 0;

    cv::Mat featuresTrain;
    cv::Mat featuresValidationI;
    cv::Mat featuresValidationQ;
    cv::Mat featuresTestI;
    cv::Mat featuresTestQ;

    timestamp_type start, end;
    cout << "Reading featuresTrain: ";
    //cv::Mat labels;
    int currentOffset = 0;
    long testOffset;

    if (parameters.count("file") > 0){

        if (parameters.count("splitToTest") > 0){
            currentOffset = atoi(parameters["splitToTest"].c_str())*(nTrain+nValI+nValQ+nTesI+nTesQ);
        }

        file = string(parameters["file"]);
        //int tmp = 0;
    //cv::Mat labels;

        get_timestamp(&start);
        importer->readBin(file,nTrain,featuresTrain,currentOffset);
        get_timestamp(&end);
        currentOffset += nTrain;
        get_timestamp(&start);
        importer->readBin(file,nValI,featuresValidationI,currentOffset);
        get_timestamp(&end);
        currentOffset += nValI;
        get_timestamp(&start);
        importer->readBin(file,nValQ,featuresValidationQ,currentOffset);
        get_timestamp(&end);
        currentOffset += nValQ;
        testOffset = currentOffset;
        get_timestamp(&start);
        importer->readBin(file,nTesI,featuresTestI,currentOffset);
        get_timestamp(&end);

        currentOffset += nTesI;
        get_timestamp(&start);
        importer->readBin(file,nTesQ,featuresTestQ,currentOffset);
        get_timestamp(&end);


    } else if (parameters.count("fileIndex") > 0){
        file = string(parameters["fileIndex"]);


        string fileIndex = string(parameters["fileIndex"]);
        string fileTrain = string(parameters["fileTrain"]);
        string fileQuery = string(parameters["fileQuery"]);

        //int tmp = 0;
        //cv::Mat labels;

        get_timestamp(&start);
        importer->readBin(fileTrain,nTrain-nValI-nValQ,featuresTrain,currentOffset);
        get_timestamp(&end);
        currentOffset += nTrain-nValI-nValQ;
        get_timestamp(&start);
        importer->readBin(fileTrain,nValI,featuresValidationI,currentOffset);
        get_timestamp(&end);
        currentOffset += nValI;
        get_timestamp(&start);
        importer->readBin(fileTrain,nValQ,featuresValidationQ,currentOffset);
        get_timestamp(&end);
        int testIndexOffset = 0;
        testOffset = 0;
        get_timestamp(&start);
        importer->readBin(fileIndex,nTesI,featuresTestI,testIndexOffset);
        get_timestamp(&end);
        int testQueryOffset = 0;
        get_timestamp(&start);
        importer->readBin(fileQuery,nTesQ,featuresTestQ,testQueryOffset);
        get_timestamp(&end);

    } else {
        cout << "error: no files specified" << endl;
        return;
    }

    //featuresValidationI.copyTo(featuresTestI);
    //featuresValidationQ.copyTo(featuresTestQ);
    //nTesI = featuresTestI.rows;
    //nTesQ = featuresTestQ.rows;
    vector<std::pair<vector<float>, vector<float> > > linearResults;

    cout << endl << "nTrain;nValI;nValQ;nTesI;nTesQ;mAcc;k;d" << endl;
    cout << nTrain << ";" << nValI << ";" << nValQ << ";" << nTesI << ";" << nTesQ << ";" << k << ";" << featuresTestI.cols << endl << endl;

    cout << "Name;Name2;TrainTime;IndexingTime;QueryTime;%Correct;MAP;avgDeltaDistance" << endl;
    for(uint i = 0; i < indexers.size(); i++){
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

        //int tmp = 0;

        vector<float> precVsLinearTmp;
        double tmpTime = 0;
        std::pair<vector<float>, vector<float> > r;
        vector<std::pair<vector<float>, vector<float> > > rAll;

        for (int j = 0; j < featuresTestQ.rows; j++){
            cv::Mat q = featuresTestQ.row(j);
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
        for (uint j = 0; j < rAll.size(); j++){
            //int kLinear = 0;
            long relAccum = 0;
            double precAccum = 0;

            for (uint m = 0; m < rAll.at(j).first.size(); m++){
                deltaDistance += rAll.at(j).second.at(m) - linearResults.at(j).second.at(m);
            }

            for (uint m = 0; m < rAll.at(j).first.size(); m++){

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
        cout << ";" <<  tmpTime/featuresTestQ.rows <<  ";" << ((double)commonElements)/(k*nTesQ) << ";" << avgPrec/(k*nTesQ) << ";" << deltaDistance << endl;

        indexers.at(i)->saveParams(indexers.at(i)->getName());

        delete indexers.at(i);
    }
}


bool loadGT(string& gtFile,vector<std::pair<vector<float>, vector<float> > >& linearResults,string &nameO,long testOffset, int nTesI, int nTesQ,int k){
    ifstream file(gtFile.c_str(), ifstream::in);
    string line, path, tmpStr;

    getline(file, line);
    string name = line;

    if (name != nameO)
    	return false;

    getline(file, line);
    int offset = atoi(line.c_str());

    if (testOffset != offset)
    	return false;

    getline(file, line);
    int nTestI = atoi(line.c_str());

    if(nTesI != nTestI)
    	return false;

    getline(file, line);
    int nRows = atoi(line.c_str());

    if(nTesQ != nRows)
    	return false;

    getline(file, line);
    int nCols = atoi(line.c_str());

    if (k > nCols)
    	return false;

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
    return true;
}

void computeGT(int argc, char *argv[]){

    //bool debug = false;
    IBinImporter* importer;

    string paramFile(argv[1]);

	map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

    string file;
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

    //if (parameters.count("debug") > 0){
	//    debug = true;
	//}

	int nTrain = atoi(parameters["nTrain"].c_str());
	int nValI = atoi(parameters["nValI"].c_str());
    int nValQ = atoi(parameters["nValQ"].c_str());
	int nTesI = atoi(parameters["nTesI"].c_str());
	int nTesQ = atoi(parameters["nTesQ"].c_str());
	vector<string> kList = StringTools::split(parameters["k"],',');
	int k = atoi(kList.at(0).c_str());

	for(uint i = 1; i < kList.size(); i++){
		if(atoi(kList.at(i).c_str()) > k)
			k = atoi(kList.at(i).c_str());
	}

    //int tmp = 0;

	cv::Mat featuresTestI;
	cv::Mat featuresTestQ;

    timestamp_type start, end;
	//cv::Mat labels;
    long currentOffset = 0;

	long nTestOffset;
	string simpleName;
	string outfileNameString;

    if (parameters.count("file") > 0){
        if (parameters.count("splitToTest") > 0){
            currentOffset = atoi(parameters["splitToTest"].c_str())*(nTrain+nValI+nValQ+nTesI+nTesQ);
        }

        file = string(parameters["file"]);
        currentOffset += nTrain + nValI + nValQ;

        nTestOffset = currentOffset;
        simpleName = file;
        uint pos = simpleName.rfind("/");

        if(pos != string::npos)
            simpleName = simpleName.substr(pos+1);

        stringstream outfileName;

        outfileName << simpleName << "_" << currentOffset << "_" << nTesI << "_" << nTesQ << "_" << k << ".gt";

        cout << outfileName.str() << endl;
        outfileNameString = outfileName.str();

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

	} else if (parameters.count("fileIndex") > 0){
        file = string(parameters["fileIndex"]);
        string fileQuery = string(parameters["fileQuery"]);

        currentOffset = 0;

        nTestOffset = currentOffset;
        simpleName = file;
        uint pos = simpleName.rfind("/");

        if(pos != string::npos)
            simpleName = simpleName.substr(pos+1);

        stringstream outfileName;

        outfileName << simpleName << "_" << currentOffset << "_" << nTesI << "_" << nTesQ << "_" << k << ".gt";

        cout << outfileName.str() << endl;
        outfileNameString = outfileName.str();

        cout << "Reading featuresTestI " << endl;
        get_timestamp(&start);
        importer->readBin(file,nTesI,featuresTestI,currentOffset);
        get_timestamp(&end);
        //currentOffset += nTesI;
        cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ " << endl;
        get_timestamp(&start);
        importer->readBin(fileQuery,nTesQ,featuresTestQ,currentOffset);
        get_timestamp(&end);
        cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
	} else {
        return;
    }

	fstream out(outfileNameString.c_str(),std::fstream::out);

    vector<std::pair<vector<float>, vector<float> > > linearResults;

    out << simpleName << endl;
    out << nTestOffset << endl;
    out << nTesI << endl;
    out << nTesQ << endl;
    out << k << endl;

	for(int i = 0; i < 1; i++){

        indexers.at(i)->indexWithTrainedParams(featuresTestI);

        //int tmp = 0;

        vector<float> precVsLinearTmp;
        double tmpTime = 0;
        std::pair<vector<float>, vector<float> > r;
        vector<std::pair<vector<float>, vector<float> > > rAll;

        for (int j = 0; j < featuresTestQ.rows; j++){
            cv::Mat q = featuresTestQ.row(j);
            get_timestamp(&start);
            r =  indexers.at(i)->knnSearchId(q,k);
            get_timestamp(&end);
            tmpTime += timestamp_diff_in_milliseconds(start, end);
            for (uint l = 0; l < r.first.size(); l++){
                out << r.first.at(l) << "," << r.second.at(l) << ";";
            }
            out << endl;
        }

		delete indexers.at(i);
	}

	vector<std::pair<vector<float>, vector<float> > > linearResultsAA;
	cout << loadGT(outfileNameString,linearResultsAA,simpleName,nTestOffset,nTesI,nTesQ,k) << endl;
}

void awesomeIndexTesterAll(int argc, char *argv[]){
    bool debug = false;
    IBinImporter* importer;

    string paramFile(argv[1]);

	map<string,string> parameters;
	vector<IIndexer*> allIndexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

    int indexToTest = -1;
    bool testAll = true;
    if (argc > 2){
        indexToTest = atoi(argv[2]);
        testAll = false;
    }

	LoadConfig::load(paramFile,parameters,allIndexers,analysers,classifiers,endpoints);

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

    long currentOffset = 0;

    if (parameters.count("debug") > 0){
	    debug = true;
	}

	int nTrain = atoi(parameters["nTrain"].c_str());
	int nValI = atoi(parameters["nValI"].c_str());
    int nValQ = atoi(parameters["nValQ"].c_str());
	int nTesI = atoi(parameters["nTesI"].c_str());
	int nTesQ = atoi(parameters["nTesQ"].c_str());

	vector<string> kList = StringTools::split(parameters["k"],',');
	int biggestK = atoi(kList.at(0).c_str());

	for(uint i = 1; i < kList.size(); i++){
		if(atoi(kList.at(i).c_str()) > biggestK)
			biggestK = atoi(kList.at(i).c_str());
	}
	//int mapAt = atoi(parameters["mapAt"].c_str());

	cv::Mat featuresTrain;
	cv::Mat featuresValidationI;
	cv::Mat featuresValidationQ;
	cv::Mat featuresTestI;
	cv::Mat featuresTestQ;

	string simpleFileName;
	string outfileNameA;
	string file;

	vector<std::pair<vector<float>, vector<float> > > linearResults;

	long testOffset;
   	timestamp_type start, end;

	if (parameters.count("file") > 0){

    	if (parameters.count("splitToTest") > 0){
			currentOffset = atoi(parameters["splitToTest"].c_str())*(nTrain+nValI+nValQ+nTesI+nTesQ);
		}

		file = string(parameters["file"]);
    	//int tmp = 0;
    	if (debug) cout << "Reading featuresTrain: ";
	//cv::Mat labels;

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
		testOffset = currentOffset;
		if (debug) cout << currentOffset << " ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestI ";
		get_timestamp(&start);
		importer->readBin(file,nTesI,featuresTestI,currentOffset);
		get_timestamp(&end);

		currentOffset += nTesI;
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ ";
		get_timestamp(&start);
		importer->readBin(file,nTesQ,featuresTestQ,currentOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;

	    simpleFileName = file;
		uint posC = simpleFileName.rfind("/");

		if (posC != string::npos)
			simpleFileName = simpleFileName.substr(posC+1);

		if (debug) cout << "Loading GT" << endl;

		stringstream outfileName;

		outfileName << simpleFileName << "_" << testOffset << "_" << nTesI << "_" << nTesQ << "_" << biggestK << ".gt";

		if (debug) cout << outfileName.str() << endl;

		outfileNameA = outfileName.str();


	} else if (parameters.count("fileIndex") > 0){
		file = string(parameters["fileIndex"]);


		string fileIndex = string(parameters["fileIndex"]);
		string fileTrain = string(parameters["fileTrain"]);
		string fileQuery = string(parameters["fileQuery"]);

    	//int tmp = 0;
    	if (debug) cout << "Reading featuresTrain: ";
		//cv::Mat labels;

	    get_timestamp(&start);
		importer->readBin(fileTrain,nTrain-nValI-nValQ,featuresTrain,currentOffset);
		get_timestamp(&end);
		currentOffset += nTrain-nValI-nValQ;
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationI ";
		get_timestamp(&start);
		importer->readBin(fileTrain,nValI,featuresValidationI,currentOffset);
		get_timestamp(&end);
		currentOffset += nValI;
	    if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationQ ";
	    get_timestamp(&start);
		importer->readBin(fileTrain,nValQ,featuresValidationQ,currentOffset);
		get_timestamp(&end);
		int testIndexOffset = 0;
		testOffset = 0;
		if (debug) cout << currentOffset << " ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestI ";
		get_timestamp(&start);
		importer->readBin(fileIndex,nTesI,featuresTestI,testIndexOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ ";
		int testQueryOffset = 0;
		get_timestamp(&start);
		importer->readBin(fileQuery,nTesQ,featuresTestQ,testQueryOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;

	    simpleFileName = file;
		uint posC = simpleFileName.rfind("/");

		if (posC != string::npos)
			simpleFileName = simpleFileName.substr(posC+1);

		if (debug) cout << "Loading GT" << endl;

		stringstream outfileName;

		outfileName << simpleFileName << "_" << testIndexOffset << "_" << nTesI << "_" << nTesQ << "_" << biggestK << ".gt";

		if (debug) cout << outfileName.str() << endl;

		outfileNameA = outfileName.str();


	} else {
		cout << "error: no files specified" << endl;
    	return;
	}

    bool gtLoaded = loadGT(outfileNameA,linearResults,simpleFileName,testOffset,nTesI,nTesQ,biggestK);

    if (!gtLoaded){
    	cout << "error loading GT: " << outfileNameA << endl;
    } else {
        if (debug) cout << "Loading GT ok" << endl;
	}
    //featuresValidationI.copyTo(featuresTestI);
    //featuresValidationQ.copyTo(featuresTestQ);
    //nTesI = featuresTestI.rows;
    //nTesQ = featuresTestQ.rows;

    vector<IIndexer*> indexers;

    if (!testAll){
        indexers.push_back(allIndexers.at(indexToTest));
    } else {
        indexers = allIndexers;
    }

    for(uint i = 0; i < indexers.size(); i++){

        if (testAll){
            indexToTest = i;
        }

        if(indexToTest == 0){
            cout << simpleFileName << endl;
            cout << "nTrain;nValI;nValQ;nTesI;nTesQ;nk;";
            for (uint ii = 0; ii < kList.size(); ii++){cout << "k" << ii+1<< ";";}
            cout << "d;offset" << endl;
            cout << nTrain << ";" << nValI << ";" << nValQ << ";" << nTesI << ";" << nTesQ << ";" << kList.size() << ";";
            for (uint ii = 0; ii < kList.size(); ii++){cout << kList.at(ii)<<";";}
            cout << featuresTestI.cols << ";" << atoi(parameters["startOffset"].c_str()) << endl;
            cout << "Index name;Name;Base name;Train time (ms);Index time (ms);Retrieval index;";
            for (uint ii = 0; ii < kList.size(); ii++){cout << "Query time (ms)@" << kList.at(ii) << ";mAcc@" << kList.at(ii) << ";MAP@" << kList.at(ii) << ";Dist@" << kList.at(ii) << ";";}
            cout << endl;
        }

    	if (debug) cout << "Training" << endl;
		get_timestamp(&start);
        indexers.at(i)->train(featuresTrain,featuresValidationQ,featuresValidationI);
		get_timestamp(&end);
		if (debug) cout << "Training ok" << endl;

		uint pos = indexers.at(i)->getName().rfind("_");
		string simpleName = indexers.at(i)->getName().substr(0,pos);

        stringstream ssAll;

		stringstream ss4;
        ss4 << std::setw(5) << std::setfill('0') << indexToTest;
        ssAll << ss4.str() << ";" << indexers.at(i)->getName() << ";" << simpleName << ";" << timestamp_diff_in_milliseconds(start, end);

        if (debug) cout << "Indexing" << endl;
		get_timestamp(&start);
        indexers.at(i)->indexWithTrainedParams(featuresTestI);
		get_timestamp(&end);
		if (debug) cout << "Indexing ok" << endl;

        //if(indexToTest == 0 || i > 0)
        ssAll << ";" << timestamp_diff_in_milliseconds(start, end);

        string training_common = ssAll.str();

		if (debug) cout << "Querying" << endl;

        int retInd = 0;

        indexers.at(i)->initRetrievalParameters();

        while (indexers.at(i)->hasNextRetrievalParameters()){

            indexers.at(i)->nextRetrievalParameters();

            stringstream ss5;
            ss5 << std::setw(5) << std::setfill('0') << retInd;
            cout << training_common << ";" << ss5.str();

            for (uint kIndex = 0; kIndex < kList.size(); kIndex++){
                //int tmp = 0;
                vector<float> precVsLinearTmp;
                double tmpTime = 0;
                std::pair<vector<float>, vector<float> > r;
                vector<std::pair<vector<float>, vector<float> > > rAll;
                int k = atoi(kList.at(kIndex).c_str());
                for (int j = 0; j < featuresTestQ.rows; j++){
                    cv::Mat q = featuresTestQ.row(j);
                    get_timestamp(&start);
                    r =  indexers.at(i)->knnSearchId(q,k);
                    get_timestamp(&end);
                    tmpTime += timestamp_diff_in_milliseconds(start, end);
                    rAll.push_back(r);
                    if (i == 0 && !gtLoaded)
                        linearResults.push_back(r);
                }
                if (debug) cout << "Querying ok" << endl;
                double deltaDistance = 0;

                double avgPrecAccum = 0;
                double accuracyAccum = 0;
                for (uint j = 0; j < rAll.size(); j++){
                    long relAccum = 0;
                    double precAccum = 0;

                    for (uint m = 0; m < rAll.at(j).first.size(); m++){
                        deltaDistance += rAll.at(j).second.at(m) - linearResults.at(j).second.at(m);
                    }
                    //for each document in rank
                    for (uint m = 0; m < rAll.at(j).first.size(); m++){
                        // Check if document m appears in the linear rank up to k
                        int isRelevant = 0;
                        for (int n = 0; n < k; n++){
                            if (rAll.at(j).first.at(m) == linearResults.at(j).first.at(n)){
                                relAccum++;
                                accuracyAccum++;
                                isRelevant = 1;
                            }
                        }
                        // Measure precision at m
                        // relAccum contains the number relevant document up to m
                        double precisionAtM = relAccum/(m+1.0);
                        if (debug){ cout << precisionAtM << " " << isRelevant << endl;}
                        if (debug){ cout << relAccum << endl; getchar();}
                        precAccum += precisionAtM * isRelevant;
                    }
                    // Accumulate results for query j
                    //do not sum if relAccum is zero (special case in MAP formula)
                    if (relAccum != 0){
                        avgPrecAccum += precAccum/(double)k;
                    }
                }
                //if(indexToTest == 0 || i > 0)
                cout <<  ";" <<  tmpTime/nTesQ <<  ";" << ((double)accuracyAccum)/((double)k*nTesQ) << ";" << avgPrecAccum/(nTesQ) << ";" << deltaDistance;

                vector<double> stats = indexers.at(i)->getStatistics();
                for (double& s: stats){
                    cout << ";" << s;
                }
                indexers.at(i)->resetStatistics();
            }
            retInd++;
            cout << endl;
        }


        stringstream ss2;

        uint pos2a = paramFile.rfind("/");
        uint pos2b = paramFile.rfind(".");

        if (pos2a == std::string::npos)
        	pos2a = -1;

		string simpleParamFileName = paramFile.substr(pos2a+1,pos2b-pos2a-1);

        ss2 << simpleParamFileName << "_" << std::setw(5) << std::setfill('0') << indexToTest << "_" << indexers.at(i)->getName();

		indexers.at(i)->saveParams(ss2.str());

		delete indexers.at(i);
	}

}

void getSAIndexStatistics(int argc, char *argv[]){
    bool debug = false;
    IBinImporter* importer;

    string paramFile(argv[1]);

	map<string,string> parameters;
	vector<IIndexer*> allIndexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

    int indexToTest = -1;
    bool testAll = true;
    if (argc > 2){
        indexToTest = atoi(argv[2]);
        testAll = false;
    }

	LoadConfig::load(paramFile,parameters,allIndexers,analysers,classifiers,endpoints);

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

    long currentOffset = 0;

    if (parameters.count("debug") > 0){
	    debug = true;
	}

	int nTrain = atoi(parameters["nTrain"].c_str());
	int nValI = atoi(parameters["nValI"].c_str());
    int nValQ = atoi(parameters["nValQ"].c_str());
	int nTesI = atoi(parameters["nTesI"].c_str());
	int nTesQ = atoi(parameters["nTesQ"].c_str());

	vector<string> kList = StringTools::split(parameters["k"],',');
	int biggestK = atoi(kList.at(0).c_str());

	for(uint i = 1; i < kList.size(); i++){
		if(atoi(kList.at(i).c_str()) > biggestK)
			biggestK = atoi(kList.at(i).c_str());
	}
	//int mapAt = atoi(parameters["mapAt"].c_str());

	cv::Mat featuresTrain;
	cv::Mat featuresValidationI;
	cv::Mat featuresValidationQ;
	cv::Mat featuresTestI;
	cv::Mat featuresTestQ;

	string simpleFileName;
	string outfileNameA;
	string file;

	vector<std::pair<vector<float>, vector<float> > > linearResults;

	long testOffset;
   	timestamp_type start, end;

	if (parameters.count("file") > 0){

    	if (parameters.count("splitToTest") > 0){
			currentOffset = atoi(parameters["splitToTest"].c_str())*(nTrain+nValI+nValQ+nTesI+nTesQ);
		}

		file = string(parameters["file"]);
    	//int tmp = 0;
    	if (debug) cout << "Reading featuresTrain: ";
	//cv::Mat labels;

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
		testOffset = currentOffset;
		if (debug) cout << currentOffset << " ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestI ";
		get_timestamp(&start);
		importer->readBin(file,nTesI,featuresTestI,currentOffset);
		get_timestamp(&end);

		currentOffset += nTesI;
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ ";
		get_timestamp(&start);
		importer->readBin(file,nTesQ,featuresTestQ,currentOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;

	    simpleFileName = file;
		uint posC = simpleFileName.rfind("/");

		if (posC != string::npos)
			simpleFileName = simpleFileName.substr(posC+1);

		if (debug) cout << "Loading GT" << endl;

		stringstream outfileName;

		outfileName << simpleFileName << "_" << testOffset << "_" << nTesI << "_" << nTesQ << "_" << biggestK << ".gt";

		if (debug) cout << outfileName.str() << endl;

		outfileNameA = outfileName.str();


	} else if (parameters.count("fileIndex") > 0){
		file = string(parameters["fileIndex"]);


		string fileIndex = string(parameters["fileIndex"]);
		string fileTrain = string(parameters["fileTrain"]);
		string fileQuery = string(parameters["fileQuery"]);

    	//int tmp = 0;
    	if (debug) cout << "Reading featuresTrain: ";
		//cv::Mat labels;

	    get_timestamp(&start);
		importer->readBin(fileTrain,nTrain-nValI-nValQ,featuresTrain,currentOffset);
		get_timestamp(&end);
		currentOffset += nTrain-nValI-nValQ;
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationI ";
		get_timestamp(&start);
		importer->readBin(fileTrain,nValI,featuresValidationI,currentOffset);
		get_timestamp(&end);
		currentOffset += nValI;
	    if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationQ ";
	    get_timestamp(&start);
		importer->readBin(fileTrain,nValQ,featuresValidationQ,currentOffset);
		get_timestamp(&end);
		int testIndexOffset = 0;
		testOffset = 0;
		if (debug) cout << currentOffset << " ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestI ";
		get_timestamp(&start);
		importer->readBin(fileIndex,nTesI,featuresTestI,testIndexOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ ";
		int testQueryOffset = 0;
		get_timestamp(&start);
		importer->readBin(fileQuery,nTesQ,featuresTestQ,testQueryOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;

	    simpleFileName = file;
		uint posC = simpleFileName.rfind("/");

		if (posC != string::npos)
			simpleFileName = simpleFileName.substr(posC+1);

		if (debug) cout << "Loading GT" << endl;

		stringstream outfileName;

		outfileName << simpleFileName << "_" << testIndexOffset << "_" << nTesI << "_" << nTesQ << "_" << biggestK << ".gt";

		if (debug) cout << outfileName.str() << endl;

		outfileNameA = outfileName.str();


	} else {
		cout << "error: no files specified" << endl;
    	return;
	}

    //featuresValidationI.copyTo(featuresTestI);
    //featuresValidationQ.copyTo(featuresTestQ);
    //nTesI = featuresTestI.rows;
    //nTesQ = featuresTestQ.rows;

    vector<IIndexer*> indexers;

    if (!testAll){
        indexers.push_back(allIndexers.at(indexToTest));
    } else {
        indexers = allIndexers;
    }

    for(uint i = 0; i < indexers.size(); i++){

        if (testAll){
            indexToTest = i;
        }

        if(indexToTest == 0){
            cout << simpleFileName << endl;
            cout << "nTrain;nValI;nValQ;nTesI;nTesQ;nk;";
            for (uint ii = 0; ii < kList.size(); ii++){cout << "k" << ii+1<< ";";}
            cout << "d;offset" << endl;
            cout << nTrain << ";" << nValI << ";" << nValQ << ";" << nTesI << ";" << nTesQ << ";" << kList.size() << ";";
            for (uint ii = 0; ii < kList.size(); ii++){cout << kList.at(ii)<<";";}
            cout << featuresTestI.cols << ";" << atoi(parameters["startOffset"].c_str()) << endl;
            cout << "Index name;Name;Base name;Train time (ms);Index time (ms);Retrieval index;";
            for (uint ii = 0; ii < kList.size(); ii++){cout << "Query time (ms)@" << kList.at(ii) << ";mAcc@" << kList.at(ii) << ";MAP@" << kList.at(ii) << ";Dist@" << kList.at(ii) << ";";}
            cout << endl;
        }

    	if (debug) cout << "Training" << endl;
		get_timestamp(&start);
        indexers.at(i)->train(featuresTrain,featuresValidationQ,featuresValidationI);
		get_timestamp(&end);
		if (debug) cout << "Training ok" << endl;

		uint pos = indexers.at(i)->getName().rfind("_");
		string simpleName = indexers.at(i)->getName().substr(0,pos);

		stringstream ss4;
        ss4 << std::setw(5) << std::setfill('0') << indexToTest;
        cout << ss4.str() << ";" << indexers.at(i)->getName() << ";" << simpleName << ";" << timestamp_diff_in_milliseconds(start, end);

        if (debug) cout << "Indexing" << endl;
		get_timestamp(&start);
        indexers.at(i)->indexWithTrainedParams(featuresValidationI);
		get_timestamp(&end);
		if (debug) cout << "Indexing ok" << endl;

        //if(indexToTest == 0 || i > 0)
        cout << ";" << timestamp_diff_in_milliseconds(start, end);

        vector<double> stats = indexers.at(i)->getStatistics();

        for (double& s: stats){
            cout << ";" << s;
        }

        cout << endl;

        stringstream ss2;

        uint pos2a = paramFile.rfind("/");
        uint pos2b = paramFile.rfind(".");

        if (pos2a == std::string::npos)
        	pos2a = -1;

		string simpleParamFileName = paramFile.substr(pos2a+1,pos2b-pos2a-1);

        ss2 << simpleParamFileName << "_" << std::setw(5) << std::setfill('0') << indexToTest << "_" << indexers.at(i)->getName();

		indexers.at(i)->saveParams(ss2.str());

		delete indexers.at(i);
	}

}


void getMoreSAIndexStatistics(int argc, char *argv[]){
    bool debug = false;
    IBinImporter* importer;

    string paramFile(argv[1]);

	map<string,string> parameters;
	vector<IIndexer*> allIndexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

    int indexToTest = -1;
    bool testAll = true;
    if (argc > 2){
        indexToTest = atoi(argv[2]);
        testAll = false;
    }

	LoadConfig::load(paramFile,parameters,allIndexers,analysers,classifiers,endpoints);

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

    long currentOffset = 0;

    if (parameters.count("debug") > 0){
	    debug = true;
	}

	int nTrain = atoi(parameters["nTrain"].c_str());
	int nValI = atoi(parameters["nValI"].c_str());
    int nValQ = atoi(parameters["nValQ"].c_str());
	int nTesI = atoi(parameters["nTesI"].c_str());
	int nTesQ = atoi(parameters["nTesQ"].c_str());

	vector<string> kList = StringTools::split(parameters["k"],',');
	int biggestK = atoi(kList.at(0).c_str());

	for(uint i = 1; i < kList.size(); i++){
		if(atoi(kList.at(i).c_str()) > biggestK)
			biggestK = atoi(kList.at(i).c_str());
	}
	//int mapAt = atoi(parameters["mapAt"].c_str());

	cv::Mat featuresTrain;
	cv::Mat featuresValidationI;
	cv::Mat featuresValidationQ;
	cv::Mat featuresTestI;
	cv::Mat featuresTestQ;

	string simpleFileName;
	string outfileNameA;
	string file;

	vector<std::pair<vector<float>, vector<float> > > linearResults;

	long testOffset;
   	timestamp_type start, end;

	if (parameters.count("file") > 0){

    	if (parameters.count("splitToTest") > 0){
			currentOffset = atoi(parameters["splitToTest"].c_str())*(nTrain+nValI+nValQ+nTesI+nTesQ);
		}

		file = string(parameters["file"]);
    	//int tmp = 0;
    	if (debug) cout << "Reading featuresTrain: ";
	//cv::Mat labels;

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
		testOffset = currentOffset;
		if (debug) cout << currentOffset << " ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestI ";
		get_timestamp(&start);
		importer->readBin(file,nTesI,featuresTestI,currentOffset);
		get_timestamp(&end);

		currentOffset += nTesI;
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ ";
		get_timestamp(&start);
		importer->readBin(file,nTesQ,featuresTestQ,currentOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;

	    simpleFileName = file;
		uint posC = simpleFileName.rfind("/");

		if (posC != string::npos)
			simpleFileName = simpleFileName.substr(posC+1);

		if (debug) cout << "Loading GT" << endl;

		stringstream outfileName;

		outfileName << simpleFileName << "_" << testOffset << "_" << nTesI << "_" << nTesQ << "_" << biggestK << ".gt";

		if (debug) cout << outfileName.str() << endl;

		outfileNameA = outfileName.str();


	} else if (parameters.count("fileIndex") > 0){
		file = string(parameters["fileIndex"]);


		string fileIndex = string(parameters["fileIndex"]);
		string fileTrain = string(parameters["fileTrain"]);
		string fileQuery = string(parameters["fileQuery"]);

    	//int tmp = 0;
    	if (debug) cout << "Reading featuresTrain: ";
		//cv::Mat labels;

	    get_timestamp(&start);
		importer->readBin(fileTrain,nTrain-nValI-nValQ,featuresTrain,currentOffset);
		get_timestamp(&end);
		currentOffset += nTrain-nValI-nValQ;
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationI ";
		get_timestamp(&start);
		importer->readBin(fileTrain,nValI,featuresValidationI,currentOffset);
		get_timestamp(&end);
		currentOffset += nValI;
	    if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationQ ";
	    get_timestamp(&start);
		importer->readBin(fileTrain,nValQ,featuresValidationQ,currentOffset);
		get_timestamp(&end);
		int testIndexOffset = 0;
		testOffset = 0;
		if (debug) cout << currentOffset << " ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestI ";
		get_timestamp(&start);
		importer->readBin(fileIndex,nTesI,featuresTestI,testIndexOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ ";
		int testQueryOffset = 0;
		get_timestamp(&start);
		importer->readBin(fileQuery,nTesQ,featuresTestQ,testQueryOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;

	    simpleFileName = file;
		uint posC = simpleFileName.rfind("/");

		if (posC != string::npos)
			simpleFileName = simpleFileName.substr(posC+1);

		if (debug) cout << "Loading GT" << endl;

		stringstream outfileName;

		outfileName << simpleFileName << "_" << testIndexOffset << "_" << nTesI << "_" << nTesQ << "_" << biggestK << ".gt";

		if (debug) cout << outfileName.str() << endl;

		outfileNameA = outfileName.str();


	} else {
		cout << "error: no files specified" << endl;
    	return;
	}

    //featuresValidationI.copyTo(featuresTestI);
    //featuresValidationQ.copyTo(featuresTestQ);
    //nTesI = featuresTestI.rows;
    //nTesQ = featuresTestQ.rows;

    vector<IIndexer*> indexers;

    if (!testAll){
        indexers.push_back(allIndexers.at(indexToTest));
    } else {
        indexers = allIndexers;
    }

    IIndexer* linear = indexers.at(0);
    IIndexer* sa = indexers.at(1);


    if (debug) cout << "Training" << endl;
    linear->train(featuresTrain,featuresValidationQ,featuresValidationI);
    sa->train(featuresTrain,featuresValidationQ,featuresValidationI);
    if (debug) cout << "Training ok" << endl;

    if (debug) cout << "Indexing" << endl;
    linear->indexWithTrainedParams(featuresValidationI);
    sa->indexWithTrainedParams(featuresValidationI);
    if (debug) cout << "Indexing ok" << endl;

    int k = 50;

    for (int j = 0; j < featuresValidationQ.rows; j++){
        cv::Mat q = featuresValidationQ.row(j);
        get_timestamp(&start);
        std::pair<vector<float>,vector<float> > r = linear->knnSearchId(q,k);
        vector<float> indeces = r.first;

        cv::Mat nn;
        for(uint n = 0; n < indeces.size(); n++){
            nn.push_back(featuresValidationI.row(indeces.at(n)));
        }

        arma::fmat qMat;
		MatrixTools::matToFMat(q, qMat);

        qMat = qMat.t();

        arma::fmat nnMat;
		MatrixTools::matToFMat(nn, nnMat);

        nnMat = nnMat.t();


        sa->getMoreStatistics(qMat,nnMat);

    }

    sa->printMoreStatistics();

}

void exportToArmaMat(int argc, char *argv[]){

	bool debug = false;
    IBinImporter* importer;

    string paramFile(argv[1]);

	map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

	//cout << allIndexers.size() << endl;

    string file, simpleFileName;
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

    long currentOffset = 0;

    if (parameters.count("debug") > 0){
	    debug = true;
	}

	int nTrain = atoi(parameters["nTrain"].c_str());
	int nValI = atoi(parameters["nValI"].c_str());
    int nValQ = atoi(parameters["nValQ"].c_str());
	int nTesI = atoi(parameters["nTesI"].c_str());
	int nTesQ = atoi(parameters["nTesQ"].c_str());
	//int k = atoi(parameters["k"].c_str());
	//int mapAt = atoi(parameters["mapAt"].c_str());

    if (parameters.count("splitToTest") > 0){
		currentOffset = atoi(parameters["splitToTest"].c_str())*(nTrain+nValI+nValQ+nTesI+nTesQ);
	}

    //int tmp = 0;

	cv::Mat featuresTrain;
	cv::Mat featuresValidationI;
	cv::Mat featuresValidationQ;
	cv::Mat featuresTestI;
	cv::Mat featuresTestQ;

    //long testOffset;
   	timestamp_type start, end;

	if (parameters.count("file") > 0){


    	if (parameters.count("splitToTest") > 0){
			currentOffset = atoi(parameters["splitToTest"].c_str())*(nTrain+nValI+nValQ+nTesI+nTesQ);
		}

		file = string(parameters["file"]);
		simpleFileName = file;
    	//int tmp = 0;
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
		//testOffset = currentOffset;
		if (debug) cout << currentOffset << " ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestI ";
		get_timestamp(&start);
		importer->readBin(file,nTesI,featuresTestI,currentOffset);
		get_timestamp(&end);

		currentOffset += nTesI;
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ ";
		get_timestamp(&start);
		importer->readBin(file,nTesQ,featuresTestQ,currentOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;

	} else if (parameters.count("fileIndex") > 0){
		file = string(parameters["fileIndex"]);
		simpleFileName = file;

		string fileIndex = string(parameters["fileIndex"]);
		string fileTrain = string(parameters["fileTrain"]);
		string fileQuery = string(parameters["fileQuery"]);

    	//int tmp = 0;
    	if (debug) cout << "Reading featuresTrain: ";
		//Mat labels;

	    get_timestamp(&start);
		importer->readBin(fileTrain,nTrain-nValI-nValQ,featuresTrain,currentOffset);
		get_timestamp(&end);
		currentOffset += nTrain-nValI-nValQ;
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationI ";
		get_timestamp(&start);
		importer->readBin(fileTrain,nValI,featuresValidationI,currentOffset);
		get_timestamp(&end);
		currentOffset += nValI;
	    if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresValidationQ ";
	    get_timestamp(&start);
		importer->readBin(fileTrain,nValQ,featuresValidationQ,currentOffset);
		get_timestamp(&end);
		int testIndexOffset = 0;
		//testOffset = 0;
		if (debug) cout << currentOffset << " ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestI ";
		get_timestamp(&start);
		importer->readBin(fileIndex,nTesI,featuresTestI,testIndexOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl << "Reading featuresTestQ ";
		int testQueryOffset = 0;
		get_timestamp(&start);
		importer->readBin(fileQuery,nTesQ,featuresTestQ,testQueryOffset);
		get_timestamp(&end);
		if (debug) cout << "ok " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;

	} else {
		cout << "error: no files specified" << endl;
    	return;
	}

	simpleFileName = paramFile;
	uint posC = simpleFileName.rfind("/");

	if (posC != string::npos)
		simpleFileName = simpleFileName.substr(posC+1);

	vector<cv::Mat> mats;

	mats.push_back(featuresTrain);
	mats.push_back(featuresValidationI);
	mats.push_back(featuresValidationQ);
	mats.push_back(featuresTestI);
	mats.push_back(featuresTestQ);

	vector<string> matNames;

	matNames.push_back("train");
	matNames.push_back("valI");
	matNames.push_back("valQ");
	matNames.push_back("testI");
	matNames.push_back("testQ");

	for (uint i = 0; i < mats.size(); i++){
		cv::Mat f = mats.at(i);
		arma::Mat<float> featuresArma((float*)f.data, f.cols, f.rows,false,true);
		stringstream sss;
		sss << "/localstore/amourao/configCondor/" << simpleFileName << "_" << matNames.at(i) << ".mat";
		cout << matNames.at(i) << " rows: " << featuresArma.n_rows << " cols: " << featuresArma.n_cols << endl;
		featuresArma.save(sss.str());
	}
}


void testSphericalHashing(int argc, char *argv[]){

	//bool debug = false;
    IBinImporter* importer;

    string paramFile(argv[1]);

	map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

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
	//int nTesI = atoi(parameters["nTesI"].c_str());
	//int nTesQ = atoi(parameters["nTesQ"].c_str());
	//int k = atoi(parameters["k"].c_str());

    //int tmp = 0;

	cv::Mat featuresTrain;
	cv::Mat featuresValidationI;
	cv::Mat featuresValidationQ;
	cv::Mat featuresTestI;
	cv::Mat featuresTestQ;

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


int getMatrixSample(int argc, char *argv[]){

    int split1(atoi(argv[1]));
    int split2(atoi(argv[2]));

    vector<cv::Mat> inVec,outVec;
    vector<string> files;
    int total = split1 + split2;

    for(int i = 3; i < argc; i++){

        string infile(argv[i]);

        cv::Mat in;
        cv::Mat inLab;

        MatrixTools::readBin(infile,in,inLab);

        inVec.push_back(in);
        inVec.push_back(inLab);

        files.push_back(infile);

    }
    MatrixTools::getRandomSample(inVec,total,outVec);
    for(int i = 0; i < files.size(); i++){

        string infile = files.at(i);
        string outfileTrain = infile + "_train";
        string outfileVal = infile + "_val";

        cv::Mat data1,data2,lab1,lab2;

        data1 = outVec.at(i*2).rowRange(0,split1);
        lab1 = outVec.at(i*2+1).rowRange(0,split1);

        data2 = outVec.at(i*2).rowRange(split1,total);
        lab2 = outVec.at(i*2+1).rowRange(split1,total);

        MatrixTools::writeBinV2(outfileTrain,data1,lab1);
        MatrixTools::writeBinV2(outfileVal,data2,lab2);

    }
    return 0;
}

int genDictionaryAndIndex(int argc, char *argv[]){

    bool debug = false;
    IBinImporter* importer;

    string paramFile(argv[1]);

	map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

    string a = "srIndexerStd";
    string b = "linearIndexer";
    FactoryIndexer * fc = FactoryIndexer::getInstance();
	IIndexer* indexer = (IIndexer*)fc->createType(a);
	IIndexer* indexerLinear = (IIndexer*)fc->createType(b);

	cv::Mat tr,val,all;
	cv::Mat trL,valL,allL;

    MatrixTools::readBin("/media/Share/data/fullFeatures/med_gist_train",tr,trL);
    MatrixTools::readBin("/media/Share/data/fullFeatures/med_gist_val",val,valL);
    MatrixTools::readBin("/media/Share/data/fullFeatures/med_gist.bin",all,allL);

	indexer->train(tr,val,val);
	indexer->index(all);
	indexer->setFlabels(allL);

	indexerLinear->train(tr,val,val);
	indexerLinear->index(all);
	indexerLinear->setFlabels(allL);


	indexer->save("med_gist883_srIndexerStd");
	indexerLinear->save("med_gist883_linearIndexer");


    return 0;
}

int testReadBin(int argc, char *argv[]){

    string type(argv[1]);
    string file(argv[2]);
    string howManyStr(argv[3]);
    int howMany = std::stoi(howManyStr);
    string out(argv[4]);

    IBinImporter* importer;

    if(type == "tiny"){
        importer = new tinyImageImporter();
    } else if(type == "billion"){
        importer = new oneBillionImporter();
    } else if(type == "nsBin"){
        importer = new MatrixTools();
    } else {
        cout << "Unknown parameter value \"type\" = " << type << endl;
        return 1;
    }
    cv::Mat featuresTrain;
    cv::Mat labels;
    importer->readBin(file,howMany,featuresTrain,0);
    MatrixTools::writeBinV2(out,featuresTrain,labels);

    return 0;
}

int main(int argc, char *argv[]){
	//awesomeIndexTester(argc, argv);
	//testSphericalHashing(argc, argv);
	//getMatrixSample(argc, argv);
    //testReadBin(argc, argv);
	//awesomeIndexTesterAll(argc, argv);

	string name(argv[0]);
	if (name == "./getBucketStats"){
		getSAIndexStatistics(argc, argv);
	} else if (name == "./getMoreBucketStats"){
		getMoreSAIndexStatistics(argc, argv);
	} else{
		awesomeIndexTesterAll(argc, argv);
	}

    return 0;
}
