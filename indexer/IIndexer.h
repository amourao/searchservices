#pragma once

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <utility>

#include <opencv2/highgui/highgui.hpp>
#include "../analyser/tools/MatrixTools.h"

#include "../commons/factory/Factory.h"

using namespace std;

#ifndef INDEXER_BASE_SAVE_PATH
#define INDEXER_BASE_SAVE_PATH "./indexer/data/"
#endif


#ifndef INDEXER_LABELS_EXTENSION
#define INDEXER_LABELS_EXTENSION ".labels"
#endif

#ifndef INDEXER_PARAMS_EXTENSION
#define INDEXER_PARAMS_EXTENSION ".params"
#endif

class IIndexer: public FactoryMethod {

public:

	virtual ~IIndexer(){}

	virtual void* createType(string &typeId) = 0;
	virtual void* createType(string &typeId, map<string,string>& params) = 0;


	virtual std::pair<vector<float>,vector<float> > knnSearchId(std::vector<float>& v, int n = 1000){
		cv::Mat vMat;
		MatrixTools::vectorToMat(v, vMat);
		return knnSearchId(vMat,n);
	}

	virtual std::pair<vector<string>,vector<float> > knnSearchName(std::vector<float>& v, int n = 1000){
		cv::Mat vMat;
		MatrixTools::vectorToMat(v, vMat);
		return knnSearchName(vMat,n);
	}

	virtual std::pair<vector<float>,vector<float> > radiusSearchId(std::vector<float>& v, double radius, int n = 1000){
		cv::Mat vMat;
		MatrixTools::vectorToMat(v, vMat);
		return radiusSearchId(vMat,radius,n);
	}

	virtual std::pair<vector<string>,vector<float> > radiusSearchName(std::vector<float>& v, double radius, int n = 1000){
		cv::Mat vMat;
		MatrixTools::vectorToMat(v, vMat);
		return radiusSearchName(vMat,radius,n);
	}


	virtual std::pair<vector<float>,vector<float> > knnSearchId(cv::Mat features, int n) = 0;

	virtual std::pair<vector<string>,vector<float> > knnSearchName(cv::Mat features, int n){
        std::pair<vector<float>,vector<float> > knnSearch = knnSearchId(features,n);
        return make_pair(idToLabels(knnSearch.first),knnSearch.second);
	}

	virtual std::pair<vector<float>,vector<float> > radiusSearchId(cv::Mat features, double radius, int n = 1000) = 0;

	virtual std::pair<vector<string>,vector<float> > radiusSearchName(cv::Mat features, double radius, int n = 1000){
        std::pair<vector<float>,vector<float> > rSearch = radiusSearchId(features,radius,n);
        return make_pair(idToLabels(rSearch.first),rSearch.second);
	}

    virtual void train(cv::Mat featuresTrain,cv::Mat featuresValidationI,cv::Mat featuresValidationQ) = 0;

	virtual void indexWithTrainedParams(cv::Mat features) = 0;

	virtual void index(cv::Mat features) = 0;

	vector<string> idToLabels(vector<float> v1){
		vector<string> result;

		for(uint i = 0; i < v1.size(); i++){
			if (labels.size() > 0){
				result.push_back(labels[v1.at(i)]);
			}
			else {
				stringstream ss;
				ss << v1.at(i);
				string s = ss.str();
				result.push_back(s);
			}
		}

		return result;
	}

	virtual void loadLabels(string basePath){

	    labels.clear();

		ifstream file(basePath.c_str(), ifstream::in);
		string line, numberOfClassesStr,id1,id2;

		getline(file, line);
		stringstream liness(line);
		getline(liness, numberOfClassesStr);
		numberOfElements = atoi(numberOfClassesStr.c_str());
		//int i = 0;
		while (getline(file, line)) {
			stringstream liness(line);
			getline(liness, id1, ',');
			getline(liness, id2);

			float labelFloat = atof(id1.c_str());
			string label = id2;

			labels.insert(std::pair<float,string>(labelFloat,label));

		}
	}

	virtual void saveLabels(string basePath){

        stringstream ss;

        ss << basePath << INDEXER_LABELS_EXTENSION;

		std::map<float,string>::iterator iter;

	    ofstream labelData(ss.str().c_str());

	    labelData << numberOfElements << endl;
	    for (iter = labels.begin(); iter != labels.end(); ++iter) {
	    	labelData << iter->first << "," << iter->second << endl;
	    }
		labelData.close();
	}

	virtual void saveParams(string basePath){

        stringstream ss;

        ss << INDEXER_BASE_SAVE_PATH << basePath << INDEXER_PARAMS_EXTENSION;

		std::map<string,string>::iterator iter;

	    ofstream labelData(ss.str().c_str());

        labelData << paramsB.size() << endl;
	    for (iter = paramsB.begin(); iter != paramsB.end(); ++iter) {
	    	labelData << iter->first << "," << iter->second << endl;
	    }
		labelData.close();
	}

	virtual bool save(string basePath) = 0;
	virtual bool load(string basePath) = 0;

	virtual string getName() = 0;

protected:

    map<string,string> paramsB;

private:


	std::map<float,string> labels;
	int numberOfElements;
};


