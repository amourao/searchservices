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

class IIndexer: public FactoryMethod {

public:

	virtual ~IIndexer(){}
	
	virtual void* createType(string &typeId) = 0;


	virtual vector<std::pair<float,float> > knnSearchId(std::vector<float>& v, int n = 1000){
		cv::Mat vMat;
		MatrixTools::vectorToMat(v, vMat);
		return knnSearchId(vMat,n);
	}

	virtual vector<std::pair<string,float> > knnSearchName(std::vector<float>& v, int n = 1000){
		cv::Mat vMat;
		MatrixTools::vectorToMat(v, vMat);
		return knnSearchName(vMat,n);
	}

	virtual vector<std::pair<float,float> > radiusSearchId(std::vector<float>& v, double radius, int n = 1000){
		cv::Mat vMat;
		MatrixTools::vectorToMat(v, vMat);
		return radiusSearchId(vMat,radius,n);
	}

	virtual vector<std::pair<string,float> > radiusSearchName(std::vector<float>& v, double radius, int n = 1000){
		cv::Mat vMat;
		MatrixTools::vectorToMat(v, vMat);
		return radiusSearchName(vMat,radius,n);
	}
	

	virtual vector<std::pair<float,float> > knnSearchId(cv::Mat features, int n) = 0;
	virtual vector<std::pair<string,float> > knnSearchName(cv::Mat features, int n) = 0;

	virtual vector<std::pair<float,float> > radiusSearchId(cv::Mat features, double radius, int n = 1000) = 0;
	virtual vector<std::pair<string,float> > radiusSearchName(cv::Mat features, double radius, int n = 1000) = 0;

	virtual void index(cv::Mat features) = 0;

	template <typename Type1, typename Type2>
	vector<std::pair<Type1,Type2> > mergeVectors(vector<Type1> v1, vector<Type2> v2){
		vector<std::pair<Type1,Type2> > result;

		for(uint i = 0; i < v1.size(); i++){
			std::pair<Type1,Type2> p (v1.at(i),v2.at(i));
			result.push_back(p);
		}

		return result;
	}

	vector<string> idToLabels(vector<float> v1){
		vector<string> result;

		for(uint i = 0; i < v1.size(); i++){
			if (labels.size() > 0)
				result.push_back(labels[v1.at(i)]);
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
		std::map<float,string>::iterator iter;

	    ofstream labelData(basePath.c_str());

	    labelData << numberOfElements << endl;
	    for (iter = labels.begin(); iter != labels.end(); ++iter) {
	    	labelData << iter->first << "," << iter->second << endl;
	    }
		labelData.close();
	}

	virtual bool save(string basePath) = 0;
	virtual bool load(string basePath) = 0;

	virtual string getName() = 0;

private:

	std::map<float,string> labels;
	int numberOfElements;
};


