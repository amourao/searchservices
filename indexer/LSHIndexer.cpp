#include "LSHIndexer.h"

//http://docs.opencv.org/2.4.3/modules/flann/doc/flann_fast_approximate_nearest_neighbor_search.html

static LSHIndexer lshIndexerFactory;

LSHIndexer::LSHIndexer(){
    FactoryIndexer::getInstance()->registerType("lshIndexer",this);
}

LSHIndexer::LSHIndexer(const string& type, map<string,string>& params){
    paramsB = params;
    typeId = type;

    hasParams = false;

    oneMinusDelta = atof(params["oneMinusDelta"].c_str());
    r = atof(params["radius"].c_str());
    trainValSplit = atof(params["trainValSplit"].c_str());

    if (params.count("memoryUpperBound") > 0){
        memoryUpperBound = atol(params["memoryUpperBound"].c_str());
    }
    else{
        memoryUpperBound = DEFAULT_MEMORY_MAX_AVAILABLE_E2LSH;
    }

    if (params.count("k") > 0){
        int k = atof(params["k"].c_str());
        int m = atof(params["m"].c_str());

        int useU = atof(params["useU"].c_str());

        int l;
        if (useU == 0)
            l = m;
        else
            l = m*(m-1.0)/2.0;
        
        int w = atof(params["w"].c_str());

        int t = atof(params["t"].c_str());
        
        int ht = atof(params["ht"].c_str());

        learnedParams.parameterR = r;
        learnedParams.successProbability = oneMinusDelta;
        learnedParams.parameterR2 = pow(r,2);
        learnedParams.useUfunctions = useU;
        learnedParams.parameterK = k;
        learnedParams.parameterM = m;
        learnedParams.parameterL = l;
        learnedParams.parameterW = w;
        learnedParams.parameterT = t;
        learnedParams.typeHT = ht;
        /*
        parameterR:3
        successProbability:0.8
        dimension:384
        parameterR2:9
        useUfunctions:1
        parameterK:2
        parameterM:3
        parameterL:3
        parameterW:4
        parameterT:100000
        typeHT:3
        */
        hasParams = true;
    }

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

void LSHIndexer::train(cv::Mat& featuresTrain,cv::Mat& featuresValidationI,cv::Mat& featuresValidationQ){
    if (!hasParams){
        PPointT* dataSet = matToPPointT(featuresTrain);
        PPointT* sampleQueries = matToPPointT(featuresValidationQ);
        learnedParams = computeOptimalParameters(r, oneMinusDelta, featuresTrain.rows, featuresTrain.cols, dataSet, featuresValidationQ.rows, sampleQueries, memoryUpperBound);
        delete dataSet;
        delete sampleQueries;
    } else {
        learnedParams.dimension = featuresTrain.cols;
    }

}

void LSHIndexer::indexWithTrainedParams(cv::Mat& features){
    PPointT* dataSet = matToPPointT(features);
    indexer =  initLSH_WithDataSet(learnedParams,features.rows,dataSet);
}

void LSHIndexer::index(cv::Mat& features){
    featuresSplit = features.rowRange(0,features.rows*trainValSplit);
    validationSplit = features.rowRange(features.rows*trainValSplit,features.rows);
    PPointT* dataSet = matToPPointT(featuresSplit);
    PPointT* sampleQueries = matToPPointT(validationSplit);
    indexer = initSelfTunedRNearNeighborWithDataSet(r, oneMinusDelta, featuresSplit.rows, featuresSplit.cols, dataSet, validationSplit.rows, sampleQueries, memoryUpperBound);
    delete dataSet;
    delete sampleQueries;
}

std::pair<vector<float>,vector<float> > LSHIndexer::knnSearchId(cv::Mat& query, int n){
    return radiusSearchId(query,-1,n);
}

std::pair<vector<float>,vector<float> > LSHIndexer::radiusSearchId(cv::Mat& query, double radius, int n){

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

    stringstream ss;

    ss << INDEXER_BASE_SAVE_PATH << basePath << INDEX_DATA_EXTENSION_E2LSH;

    ofstream labelData(ss.str().c_str());

    labelData << "parameterR:" << learnedParams.parameterR << endl;
    labelData << "successProbability:" << learnedParams.successProbability << endl;
    labelData << "dimension:" << learnedParams.dimension << endl;
    labelData << "parameterR2:" << learnedParams.parameterR2 << endl;
    labelData << "useUfunctions:" << learnedParams.useUfunctions << endl;
    labelData << "parameterK:" << learnedParams.parameterK << endl;
    labelData << "parameterM:" << learnedParams.parameterM << endl;
    labelData << "parameterL:" << learnedParams.parameterL << endl;
    labelData << "parameterW:" << learnedParams.parameterW << endl;
    labelData << "parameterT:" << learnedParams.parameterT << endl;
    labelData << "typeHT:" << learnedParams.typeHT << endl;

    labelData.close();
/*
  RealT parameterR; // parameter R of the algorithm.
  RealT successProbability; // the success probability 1-\delta
  IntT dimension; // dimension of points.
  RealT parameterR2; // = parameterR^2

  // Whether to use <u> hash functions instead of usual <g>
  // functions. When this flag is set to TRUE, <u> functions are
  // generated (which are roughly k/2-tuples of LSH), and a <g>
  // function is a pair of 2 different <u> functions.
  BooleanT useUfunctions;

  IntT parameterK; // parameter K of the algorithm.

  // parameter M (# of independent tuples of LSH functions)
  // if useUfunctions==TRUE, parameterL = parameterM * (parameterM - 1) / 2
  // if useUfunctions==FALSE, parameterL = parameterM
  IntT parameterM;

  IntT parameterL; // parameter L of the algorithm.
  RealT parameterW; // parameter W of the algorithm.
  IntT parameterT; // parameter T of the algorithm.

  // The type of the hash table used for storing the buckets (of the
  // same <g> function).
  IntT typeHT;
*/
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

