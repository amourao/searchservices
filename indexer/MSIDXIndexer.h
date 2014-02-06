//K-NN Similarity Queries for Multiple Sort Indexing for OpenCV
//Author: Andre Mourao
//Based on the original code and on the article pseudo-code.

//---------------------------------------------------
//K-NN Similarity Queries for Multiple Sort Indexing
//Version 1.0 - Information Technologies Institute @ 2012
//---------------------------------------------------

//If you plan to use MSIDX in your research, please cite the following article:
//E. Tiakas, D. Rafailidis, A. Dimou, P. Darras, "MSIDX: Multi-Sort Indexing for
//Efficient Content-based Image Search and Retrieval", IEEE Trans. on Multimedia,
//accepted for publication.


#pragma once

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>

#include "IIndexer.h"
#include "FactoryIndexer.h"

using namespace std;

#ifndef INDEX_DATA_EXTENSION_MSIDX
#define INDEX_DATA_EXTENSION_MSIDX ".xml"
#endif


#ifndef INDEX_CARDINALITY_EXTENSION_MSIDX
#define INDEX_CARDINALITY_EXTENSION_MSIDX ".txt"
#endif


struct sortCardinalities
{
    inline bool operator() (const pair<int,int>& pair1, const pair<int,int>& pair2)
    {
        return (pair1.second > pair2.second);
    }
};

class sortByCardinality {
      vector<pair<int,int> > cardinalities_;
public:
      sortByCardinality(vector<pair<int,int> > cardinalities) : cardinalities_(cardinalities) {}
      bool operator()(const pair<int,Mat>& p1, const pair<int,Mat>& p2) const {
      		Mat mat1 = p1.second;
      		Mat mat2 = p2.second;
      		for (uint i = 0; i < cardinalities_.size(); i++){
      			int colId = cardinalities_.at(i).first;
      			if (mat1.at<float>(0,colId) > mat2.at<float>(0,colId))
      				return true;
      			if (mat1.at<float>(0,colId) < mat2.at<float>(0,colId))
      				return false;
      		}
            return false;
      }
};

class compareMatDists{
     public:
     bool operator()(const pair<float,float>& pair1, const pair<float,float>& pair2){
     	if (pair1.second == pair2.second)
     		return pair1.first > pair2.first;//optional, sorts by id if equal
        return (pair1.second > pair2.second);
     }
 };

class MSIDXIndexer: public IIndexer {

public:

	MSIDXIndexer();
	MSIDXIndexer(string& type, map<string,string> params);
	~MSIDXIndexer();

	void* createType(string &typeId);

	void index(cv::Mat features);

	vector<std::pair<float,float> > knnSearchId(cv::Mat name, int k);
	vector<std::pair<string,float> > knnSearchName(cv::Mat name, int k);

	vector<std::pair<float,float> > radiusSearchId(cv::Mat name, double radius, int k);
	vector<std::pair<string,float> > radiusSearchName(cv::Mat name, double radius, int k);

	bool save(string basePath);
	bool load(string basePath);

	string getName();

private:

	std::vector<pair<int,int> > preProcessCardinality(cv::Mat& features);
	std::vector<pair<int,cv::Mat> > preProcessMultisortFeatures(cv::Mat& features);
	int getIndexBinarySearch(cv::Mat& query);


	int compareToMatCardinality(const cv::Mat& mat1, const cv::Mat& mat2);

	string type;
	//cv::Mat indexData;
	vector<pair<int,cv::Mat> > featuresList;
	std::vector<pair<int,int> > cardinalitiesCols;

	int n; //Total number of Objects (cardinality)
	int d; //Dimensionality of the Objects
	double w; //Percentage of database to search

};
