#include "MSIDXIndexer.h"

static MSIDXIndexer MSIDXIndexerFactory;

MSIDXIndexer::MSIDXIndexer(){
	FactoryIndexer::getInstance()->registerType("MSIDXIndexer",this);
}

MSIDXIndexer::MSIDXIndexer(string& _type){
	type = _type;
}

MSIDXIndexer::MSIDXIndexer(string& _type, map<string,string>& params){
	type = _type;
	w = atof(params["w"].c_str());
}

MSIDXIndexer::~MSIDXIndexer(){

}

void* MSIDXIndexer::createType(string &typeId, map<string,string>& params){
    return new MSIDXIndexer(typeId,params);
}

void* MSIDXIndexer::createType(string &typeId){
	if (typeId == "MSIDXIndexer"){
		return new MSIDXIndexer(typeId);
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}


void MSIDXIndexer::train(cv::Mat featuresTrain,cv::Mat featuresValidationI,cv::Mat featuresValidationQ){
    cardinalitiesCols = preProcessCardinality(featuresTrain);
}

void MSIDXIndexer::indexWithTrainedParams(cv::Mat features){
    if (cardinalitiesCols.size() > 0)
        featuresList = preProcessMultisortFeatures(features);
}

void MSIDXIndexer::index(cv::Mat features){
	cardinalitiesCols = preProcessCardinality(features);
	featuresList = preProcessMultisortFeatures(features);
}

std::vector<pair<int,int> > MSIDXIndexer::preProcessCardinality(cv::Mat& features){
	std::vector<pair<int,int> > cardinalities(features.cols);

	Mat featuresT;

	transpose(features,featuresT);

	for (int col = 0; col < features.cols; col++){
		std::set<float> uniqueValues;
		for (int row = 0; row < features.rows; row++){
			uniqueValues.insert(featuresT.at<float>(col,row));
		}
		cardinalities.at(col) = std::make_pair (col,uniqueValues.size());
	}

	std::sort(cardinalities.begin(),cardinalities.end(),sortCardinalities());

	return cardinalities;

}

std::vector<pair<int,cv::Mat> > MSIDXIndexer::preProcessMultisortFeatures(cv::Mat& features){
	std::vector<pair<int,cv::Mat> > sortedIndex(features.rows);

	for (int row = 0; row < features.rows; row++){
		sortedIndex.at(row) = std::make_pair (row,features.row(row));
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

std::pair<vector<float>,vector<float> > MSIDXIndexer::knnSearchId(cv::Mat query, int k){
	std::vector<float> indicesFloat;
	std::vector<float> dists;

	uint pos = getIndexBinarySearch(query);

	std::priority_queue<pair<float,float>,std::vector<pair<float,float> > ,compareMatDists> H;

	int n = featuresList.size();

    int searchW = (w*n)/2.0;

	searchW = std::min(searchW,n/2);

	k = std::min(k,n);

	int npos = 0;

	for (int j = 0; j <= searchW; j++){
		npos = pos - j;
		if (npos >= 0 && j != 0){
			Mat oa = featuresList.at(npos).second;
			float dist = pow(cv::norm(query-oa,NORM_L2),2); //equal to Linear flann metric
			H.push(std::make_pair (featuresList.at(npos).first,dist));
		}
		npos = pos + j;
		if (npos < n){
			Mat ob = featuresList.at(npos).second;
			float dist = pow(cv::norm(query-ob,NORM_L2),2);
			H.push(std::make_pair (featuresList.at(npos).first,dist));
		}
	}
	int newN = min(k,(int)H.size());
	indicesFloat.reserve(newN);
	dists.reserve(newN);
	for (int i = 0; i < newN; i++){
		pair<float,float> p = H.top();
		indicesFloat.push_back(p.first);
		dists.push_back(p.second);
		H.pop();
	}
	return make_pair(indicesFloat,dists);
}

std::pair<vector<float>,vector<float> > MSIDXIndexer::radiusSearchId(cv::Mat query, double radius, int k){
	std::vector<float> indicesFloat;
	std::vector<float> dists;
	return make_pair(indicesFloat,dists);
}


bool MSIDXIndexer::save(string basePath){
	return false;
}

bool MSIDXIndexer::load(string basePath){
	return false;
}

string MSIDXIndexer::getName(){
	return type;
}
