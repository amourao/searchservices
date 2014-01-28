#include "MSIDXIndexer.h"

static MSIDXIndexer MSIDXIndexerFactory;

MSIDXIndexer::MSIDXIndexer(){
	FactoryIndexer::getInstance()->registerType("MSIDXIndexer",this);
}

MSIDXIndexer::MSIDXIndexer(string& _type, int _w){
	type = _type;
	w = _w;
}

MSIDXIndexer::~MSIDXIndexer(){

}

void* MSIDXIndexer::createType(string &typeId){
	if (typeId == "MSIDXIndexer"){
		return new MSIDXIndexer(typeId,10);
	}  
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void MSIDXIndexer::index(cv::Mat features){
	cardinalitiesCols = preProcessCardinality(features);
	featuresList = preProcessMultisortFeatures(features);
}

std::vector<pair<int,int> > MSIDXIndexer::preProcessCardinality(cv::Mat& features){
	std::vector<pair<int,int> > cardinalities;

	for (int col = 0; col < features.cols; col++){
		std::set<float> uniqueValues;
		for (int row = 0; row < features.rows; row++){
			uniqueValues.insert(features.at<float>(row,col));
		}
		cardinalities.push_back(std::make_pair (col,uniqueValues.size()));
	}

	std::sort(cardinalities.begin(),cardinalities.end(),sortCardinalities());

	return cardinalities;

}

std::vector<pair<int,cv::Mat> > MSIDXIndexer::preProcessMultisortFeatures(cv::Mat& features){
	std::vector<pair<int,cv::Mat> > sortedIndex;

	for (int row = 0; row < features.rows; row++){
		sortedIndex.push_back(std::make_pair (row,features.row(row)));
	}

	std::sort(sortedIndex.begin(),sortedIndex.end(),sortByCardinality(cardinalitiesCols));

	return sortedIndex;

}


int MSIDXIndexer::getIndexBinarySearch(cv::Mat& query){
	uint imin = 0;
	uint imax = featuresList.size();
	uint imid = 0;
	while (imax >= imin){
		imid = imin + ((imax-imin)/2);
		Mat elem = featuresList.at(imid).second;
		int result = compareToMatCardinality(query,elem);
		if (result == 0)
			return imid;
		else if (result > 0)
			imax = imid - 1;
		else
			imin = imid + 1;
	}
	return imid;

}

int MSIDXIndexer::compareToMatCardinality(const cv::Mat& mat1, const cv::Mat& mat2){
    for (uint i = 0; i < cardinalitiesCols.size(); i++){
		int colId = cardinalitiesCols.at(i).first;
		if (mat1.at<float>(0,colId) > mat2.at<float>(0,colId))
			return 1;
		if (mat1.at<float>(0,colId) < mat2.at<float>(0,colId))
			return -1;
    }
    return 0;
}



 

 

vector<std::pair<float,float> > MSIDXIndexer::knnSearchId(cv::Mat query, int k){
	std::vector<float> indicesFloat;
	std::vector<float> dists;
	
	uint pos = getIndexBinarySearch(query);

	std::priority_queue<pair<float,float>,std::vector<pair<float,float> > ,compareMatDists> H;

	int n = featuresList.size();

	w = std::min(w,n/2+1);
	k = std::min(k,std::min(n,2*w-1));

	int npos = 0;
	for (int j = 0; j <= w; j++){
		npos = pos - j;
		if (npos >= 0 && j != 0){
			Mat oa = featuresList.at(npos).second;
			float dist = pow(cv::norm(query-oa,NORM_L2),2);
			H.push(std::make_pair (featuresList.at(npos).first,dist));
		}
		npos = pos + j;
		if (npos < n){
			Mat ob = featuresList.at(npos).second;
			float dist = pow(cv::norm(query-ob,NORM_L2),2);
			H.push(std::make_pair (featuresList.at(npos).first,dist));
		}
	}
	for (int i = 0; i < k; i++){
		pair<float,float> p = H.top();
		indicesFloat.push_back(p.first);
		dists.push_back(p.second);
		H.pop();
	}
	return mergeVectors(indicesFloat,dists);
}

vector<std::pair<string,float> > MSIDXIndexer::knnSearchName(cv::Mat 
	query, int k){
	std::vector<float> indicesFloat;
	std::vector<float> dists;
	return mergeVectors(idToLabels(indicesFloat),dists);
}

vector<std::pair<float,float> > MSIDXIndexer::radiusSearchId(cv::Mat query, double radius, int k){
	std::vector<float> indicesFloat;
	std::vector<float> dists;
	return mergeVectors(indicesFloat,dists);
}

vector<std::pair<string,float> > MSIDXIndexer::radiusSearchName(cv::Mat query, double radius, int k){
	std::vector<float> indicesFloat;
	std::vector<float> dists;
	return mergeVectors(idToLabels(indicesFloat),dists);
}

bool MSIDXIndexer::save(string basePath){
	return true;
}

bool MSIDXIndexer::load(string basePath){
	return true;
}

string MSIDXIndexer::getName(){
	return "MSIDXIndexer";
}
