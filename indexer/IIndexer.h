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

#ifndef INDEXER_FLABELS_EXTENSION
#define INDEXER_FLABELS_EXTENSION ".labels.bin"
#endif

#ifndef INDEXER_PARAMS_EXTENSION
#define INDEXER_PARAMS_EXTENSION ".params"
#endif

class IIndexer: public FactoryMethod {

public:

	virtual ~IIndexer(){}

	virtual void* createType(string &typeId) = 0;
	virtual void* createType(string &typeId, map<string,string>& params) = 0;


	virtual std::pair<vector<float>,vector<float> > knnSearchId(std::vector<float>& v, int n = 1000, int labels = -1){
		cv::Mat vMat;
		MatrixTools::vectorToMat(v, vMat);
		if (indexToLabels.empty() || labels == -1){
			return knnSearchId(vMat,n);
		} else {
			std::pair<vector<float>,vector<float> > res = knnSearchId(vMat,n);
			return getAltLabels(res,labels);
		}
	}

	virtual std::pair<vector<string>,vector<float> > knnSearchName(std::vector<float>& v, int n = 1000,int labels = -1){
		std::pair<vector<float>,vector<float> > knnSearch = knnSearchId(v,n,labels);
        return make_pair(idToLabels(knnSearch.first),knnSearch.second);
	}

	virtual std::pair<vector<float>,vector<float> > radiusSearchId(std::vector<float>& v, double radius, int n = 1000,int labels = -1){
		cv::Mat vMat;
		MatrixTools::vectorToMat(v, vMat);
		if (indexToLabels.empty() || labels == -1){
			return radiusSearchId(vMat,n);
		} else {
			std::pair<vector<float>,vector<float> > res = radiusSearchId(vMat,n);
			return getAltLabels(res,labels);
		}
	}

	virtual std::pair<vector<string>,vector<float> > radiusSearchName(std::vector<float>& v, double radius, int n = 1000,int labels = -1){
		std::pair<vector<float>,vector<float> > knnSearch = radiusSearchId(v,n,labels);
        return make_pair(idToLabels(knnSearch.first),knnSearch.second);
	}

	virtual std::pair<vector<float>,vector<float> > knnSearchId(cv::Mat& features, int n){
        arma::fmat vMat;
		MatrixTools::matToFMat(features, vMat);
        return knnSearchId(vMat,n);
	}

	virtual std::pair<vector<float>,vector<float> > knnSearchId(arma::fmat& features, int n){
        cv::Mat vMat;
		MatrixTools::fmatToMat(features, vMat);
        return knnSearchId(vMat,n);
	}

	virtual std::pair<vector<string>,vector<float> > knnSearchName(cv::Mat& features, int n){
        std::pair<vector<float>,vector<float> > knnSearch = knnSearchId(features,n);
        return make_pair(idToLabels(knnSearch.first),knnSearch.second);
	}

	virtual std::pair<vector<float>,vector<float> > radiusSearchId(cv::Mat& features, double radius, int n = 1000){
        arma::fmat vMat;
		MatrixTools::matToFMat(features, vMat);
        return radiusSearchId(vMat,radius,n);
	}

	virtual std::pair<vector<float>,vector<float> > radiusSearchId(arma::fmat& features, double radius, int n = 1000){
        cv::Mat vMat;
		MatrixTools::fmatToMat(features, vMat);
        return radiusSearchId(vMat,radius,n);
	}

	virtual std::pair<vector<string>,vector<float> > radiusSearchName(cv::Mat& features, double radius, int n = 1000){
        std::pair<vector<float>,vector<float> > rSearch = radiusSearchId(features,radius,n);
        return make_pair(idToLabels(rSearch.first),rSearch.second);
	}

	virtual std::pair<vector<string>,vector<float> > radiusSearchName(arma::fmat& features, double radius, int n = 1000){
        std::pair<vector<float>,vector<float> > rSearch = radiusSearchId(features,radius,n);
        return make_pair(idToLabels(rSearch.first),rSearch.second);
	}

    virtual void train(cv::Mat& featuresTrain,cv::Mat& featuresValidationI,cv::Mat& featuresValidationQ){
        arma::fmat vMat1,vMat2,vMat3;
		MatrixTools::matToFMat(featuresTrain, vMat1);
		MatrixTools::matToFMat(featuresValidationI, vMat2);
		MatrixTools::matToFMat(featuresValidationQ, vMat3);
        train(vMat1,vMat2,vMat3);
    }

    virtual void train(arma::fmat& featuresTrain,arma::fmat& featuresValidationI,arma::fmat& featuresValidationQ){
        cv::Mat vMat1,vMat2,vMat3;
		MatrixTools::fmatToMat(featuresTrain, vMat1);
		MatrixTools::fmatToMat(featuresValidationI, vMat2);
		MatrixTools::fmatToMat(featuresValidationQ, vMat3);
        train(vMat1,vMat2,vMat3);
    }

	virtual void indexWithTrainedParams(cv::Mat& features){
        arma::fmat vMat;
		MatrixTools::matToFMat(features, vMat);
        indexWithTrainedParams(vMat);
    }

	virtual void indexWithTrainedParams(arma::fmat& features){
        cv::Mat vMat;
		MatrixTools::fmatToMat(features, vMat);
        indexWithTrainedParams(vMat);
    }

	virtual void index(cv::Mat& features){
        arma::fmat vMat;
		MatrixTools::matToFMat(features, vMat);
        index(vMat);
	}

	virtual void index(arma::fmat& features){
        cv::Mat vMat;
		MatrixTools::fmatToMat(features, vMat);
        index(vMat);
	}

	virtual int addToIndexLive(arma::fmat& features){
        cv::Mat vMat;
		MatrixTools::fmatToMat(features, vMat);
        return addToIndexLive(vMat);
	}

	virtual int addToIndexLive(cv::Mat& features){
        arma::fmat vMat;
		MatrixTools::matToFMat(features, vMat);
        return addToIndexLive(vMat);
	}

	vector<string> idToLabels(vector<float> v1){
		vector<string> result;

		for(uint i = 0; i < v1.size(); i++){
			if ((chosenLabels >= 0) && (chosenLabels < allLabels.size()) && allLabels.at(chosenLabels).size() > 0){
				result.push_back(allLabels.at(chosenLabels)[v1.at(i)]);
			} else {
				stringstream ss;
				ss << v1.at(i);
				string s = ss.str();
				result.push_back(s);
			}
		}

		return result;
	}

	virtual void loadLabels(string basePath){
		allLabels.clear();

		stringstream ss;

        ss << INDEXER_BASE_SAVE_PATH << basePath << INDEXER_LABELS_EXTENSION;

		ifstream file(ss.str().c_str(), ifstream::in);
		string line, numberOfClassesStr,id1,id2;

		getline(file, line);
		stringstream liness(line);
		getline(liness, numberOfClassesStr);
		int labelsCount = atoi(numberOfClassesStr.c_str());

		for (int i = 0; i < labelsCount; i++){
			std::map<float,string> labels;
			getline(file, line);
			stringstream liness(line);
			getline(liness, numberOfClassesStr);
			int numberOfElements = atoi(numberOfClassesStr.c_str());
			//int i = 0;
			for (int j = 0; j < numberOfElements; j++){
				getline(file, line);
				stringstream liness(line);
				getline(liness, id1, ',');
				getline(liness, id2);

				float labelFloat = atof(id1.c_str());
				string label = id2;

				labels.insert(std::pair<float,string>(labelFloat,label));

			}
			allLabels.push_back(labels);
		}

		stringstream ss2;

        ss2 << INDEXER_BASE_SAVE_PATH << basePath << INDEXER_FLABELS_EXTENSION;

		string filePath = ss2.str();
		cv::Mat a;
		MatrixTools::readBinV2(filePath,indexToLabels,a);
	}

	virtual void addLabelLive(int originalIndex, vector<string> labels){
		for (int i = 0; i < allLabels.size(); i++){
            cout << i << " " << originalIndex << " " << labels.at(i) << endl;
			allLabels[i].insert(std::pair<float,string>(originalIndex,labels.at(i)));
		}
	}

	virtual void saveLabels(string basePath){

        stringstream ss;

        ss << INDEXER_BASE_SAVE_PATH << basePath << INDEXER_LABELS_EXTENSION;
        ofstream labelData(ss.str().c_str());

        labelData << allLabels.size() << endl;

        for (uint i = 0; i < allLabels.size(); i++){
        	std::map<float,string> labels = allLabels.at(i);
			std::map<float,string>::iterator iter;

		    labelData << labels.size() << endl;
		    for (iter = labels.begin(); iter != labels.end(); ++iter) {
		    	labelData << iter->first << "," << iter->second << endl;
		    }
		}
		labelData.close();

		stringstream ss2;

        ss2 << INDEXER_BASE_SAVE_PATH << basePath << INDEXER_FLABELS_EXTENSION;

		string filePath = ss2.str();
		cv::Mat a;
		MatrixTools::writeBinV2(filePath,indexToLabels,a);
	}

	virtual void chooseLabels(int i){
		chosenLabels = i;
	}

	virtual void setFlabels(Mat& flabels){
		indexToLabels = flabels;
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
    cv::Mat indexToLabels;

private:

	std::pair<vector<float>,vector<float> > getAltLabels(std::pair<vector<float>,vector<float> >& results, int lIndex){
		vector<float> f = results.first;
		vector<float> altResult;
		for(uint i = 0; i < f.size(); i++){
			altResult.push_back(indexToLabels.at<float>(f[i],lIndex));
		}
		return make_pair(altResult,results.second);
	}

	vector<std::map<float,string> > allLabels;

	int chosenLabels;
};


