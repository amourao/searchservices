#include "LSHIndexer.h"

//http://docs.opencv.org/2.4.3/modules/flann/doc/flann_fast_approximate_nearest_neighbor_search.html

static LSHIndexer lshIndexerFactory;

LSHIndexer::LSHIndexer(){
	FactoryIndexer::getInstance()->registerType("lshIndexer",this);
}

LSHIndexer::LSHIndexer(const string& type, map<string,string>& params){
	paramsB = params;
	typeId = type;

	oneMinusDelta = atof(params["oneMinusDelta"].c_str());
	r = atof(params["radius"].c_str());
	trainValSplit = atof(params["trainValSplit"].c_str());

    if (params.count("memoryUpperBound") > 0)
        memoryUpperBound = atol(params["memoryUpperBound"].c_str());
    else
        memoryUpperBound = DEFAULT_MEMORY_MAX_AVAILABLE_E2LSH;

}

LSHIndexer::LSHIndexer(const string& type){
	typeId = type;
}


LSHIndexer::~LSHIndexer(){

}

void* LSHIndexer::createType(string &type, map<string,string>& params){
	return new LSHIndexer(type,params);
}

void* LSHIndexer::createType(string &type){
	if (type == "lshIndexer"){
		return new LSHIndexer(type);
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void LSHIndexer::train(cv::Mat featuresTrain,cv::Mat featuresValidationI,cv::Mat featuresValidationQ){
    PPointT* dataSet = matToPPointT(featuresTrain);
    PPointT* sampleQueries = matToPPointT(featuresValidationQ);
    learnedParams = computeOptimalParameters(r, oneMinusDelta, featuresTrain.rows, featuresTrain.cols, dataSet, featuresValidationQ.rows, sampleQueries, memoryUpperBound);
}

void LSHIndexer::indexWithTrainedParams(cv::Mat features){
    PPointT* dataSet = matToPPointT(features);
    indexer =  initLSH_WithDataSet(learnedParams,features.rows,dataSet);
}

void LSHIndexer::index(cv::Mat features){
    featuresSplit = features.rowRange(0,features.rows*trainValSplit);
    validationSplit = features.rowRange(features.rows*trainValSplit,features.rows);
    PPointT* dataSet = matToPPointT(featuresSplit);
    PPointT* sampleQueries = matToPPointT(validationSplit);
    indexer = initSelfTunedRNearNeighborWithDataSet(r, oneMinusDelta, featuresSplit.rows, featuresSplit.cols, dataSet, validationSplit.rows, sampleQueries, memoryUpperBound);
}

std::pair<vector<float>,vector<float> > LSHIndexer::knnSearchId(const cv::Mat query, const int n){
	return radiusSearchId(query,-1,n);
}

std::pair<vector<float>,vector<float> > LSHIndexer::radiusSearchId(const cv::Mat query, const double radius, const int n){

	/*
	INDICIES:

	Int32T *markedPointsIndeces;
    // the size of <markedPoints> and of <markedPointsIndeces>
    IntT sizeMarkedPoints;


    DISTANCES:

    RealT distance(IntT dimension, PPointT p1, PPointT p2){
    RealT result = 0;

    for (IntT i = 0; i < dimension; i++){
    result += SQR(p1->coordinates[i] - p2->coordinates[i]);
    }

    return SQRT(result);
    }
    */

    int d = query.cols;
    PPointT *result = (PPointT*)MALLOC(n * sizeof(*result));

    PPointT* queryPoints = matToPPointT(query);
    PPointT queryPoint = queryPoints[0];

    int tmpN = n;

    int newN = getRNearNeighbors(indexer,queryPoint,result,tmpN);

    vector<float> indicesFloat;
    vector<float> dists;

    sortFinalCandidates(result,queryPoint,indicesFloat,dists,d,n,newN);

	return make_pair(indicesFloat,dists);
}

void LSHIndexer::sortFinalCandidates(PPointT *result,PPointT queryPoint, vector<float>& indicesFloat, vector<float>& dists, int d, int n, int newN){
    PPointAndRealTStructT *distToNN = (PPointAndRealTStructT*)MALLOC(newN * sizeof(*distToNN));

    for(int p = 0; p < newN; p++){
        distToNN[p].ppoint = result[p];
        distToNN[p].real = SQR(distance(d, queryPoint, result[p]));
	}
    qsort(distToNN, newN, sizeof(*distToNN), comparePPointAndRealTStructT);

    for(int j = 0; j < min(newN, n); j++){
        indicesFloat.push_back(distToNN[j].ppoint->index);
        dists.push_back(distToNN[j].real);
	}
}


bool LSHIndexer::save(const string basePath){
	return true;
}

bool LSHIndexer::load(const string basePath){
	return true;
}

string LSHIndexer::getName(){
	return typeId;
}

/*
typedef struct _PointT {
  //IntT dimension;
  IntT index; // the index of this point in the dataset list of points
  RealT *coordinates;
  RealT sqrLength; // the square of the length of the vector
} PointT, *PPointT;
*/


PPointT* LSHIndexer::matToPPointT(const Mat& input){
    int pointsDimension = input.cols;
    PPointT* dataSetPoints = (PPointT*)MALLOC(input.rows * sizeof(PPointT));


    for (int i = 0; i < input.rows; i++){
        PPointT p;
        RealT sqrLength = 0;
        p = (PPointT)MALLOC(sizeof(PointT));
        p->coordinates = (RealT*)MALLOC(pointsDimension * sizeof(RealT));
        for(int d = 0; d < pointsDimension; d++){
            p->coordinates[d] = input.at<float>(i,d);
            sqrLength += SQR(p->coordinates[d]);
        }
        p->index = i;
        p->sqrLength = sqrLength;

        dataSetPoints[i] = p;
    }
    return dataSetPoints;
}

Mat LSHIndexer::pPointsTToMat(const PPointT *(&input), int d, int n){
    Mat result(n,d,CV_32F);

    for(int i = 0; i < n; i++)
        for(int j = 0; j < d; j++)
            result.at<float>(i,j) = input[i]->coordinates[j];
}

vector<float> LSHIndexer::pPointsTToIndeces(PPointT *(&input), int n){
    vector<float> result;

    for(int i = 0; i < n; i++)
        result.push_back(input[i]->index);

    return result;
}


vector<float> LSHIndexer::pPointsTToDists(PPointT *(&input), PPointT query, int d, int n){
    vector<float> result;

    for(int i = 0; i < n; i++){
        result.push_back(distance(d, input[i], query));
    }

    return result;
}

