#include "SphericalHashIndexer.h"

//http://docs.opencv.org/2.4.3/modules/flann/doc/flann_fast_approximate_nearest_neighbor_search.html

static SphericalHashIndexer SphericalHashIndexerFactory;

SphericalHashIndexer::SphericalHashIndexer(){
    FactoryIndexer::getInstance()->registerType("sphericalHashIndexer",this);
}

SphericalHashIndexer::SphericalHashIndexer(const string& type, map<string,string>& params){
    paramsB = params;
    typeId = type;

    // desired portion of training set inside of one hyper-sphere
    //#define INCLUDING_RATIO			0.5
    // desired portion of training set inside of two hyper-spheres
    //#define OVERLAP_RATIO			0.25

    // e_m and e_s
    //#define EPSILON_MEAN			0.10
    //#define EPSILON_STDDEV			0.15

    //#define MAX_NUM_ITERATIONS		50

	bCodeLen = 64;
	trainSamplesCount = 1000;
	incRatio = 0.5;
	overRatio = 0.25;
	epsMean = 0.1;
	epsStdDev = 0.15;
	maxItr = 50;


    if (paramsB["algorithm"] == "lsh"){
        isLSH = true;
    }
}

SphericalHashIndexer::SphericalHashIndexer(const string& type){
    typeId = type;

}


SphericalHashIndexer::~SphericalHashIndexer(){

}

void* SphericalHashIndexer::createType(string &type, map<string,string>& params){
    return new SphericalHashIndexer(type,params);
}

void* SphericalHashIndexer::createType(string &type){
    if (type == "sphericalHashIndexer"){
        return new SphericalHashIndexer(type);
    }
    cerr << "Error registering type from constructor (this should never happen)" << endl;
    return NULL;
}

void SphericalHashIndexer::train(cv::Mat featuresTrain,cv::Mat featuresValidationI,cv::Mat featuresValidationQ){


}

void SphericalHashIndexer::indexWithTrainedParams(cv::Mat features){
    index(features);
}

void SphericalHashIndexer::index(cv::Mat features){

    matToPoints(features,dps);

    dataCenter = new REAL_TYPE[ dps.dim ];
	// compute mean position of data points
	dps.Compute_Center( dataCenter);

    nP = features.rows;
    dims = features.cols;

    if (isLSH){
        lsh.Initialize(dps.dim,bCodeLen);
	} else {
        sh.Initialize(&dps,dps.dim,bCodeLen,trainSamplesCount,maxItr,incRatio,overRatio,epsMean,epsStdDev);
        sh.Set_Spheres();
    }

    bCodeData = new bitset<MAX_BCODE_LEN> [nP];

    if (isLSH){
        Do_ZeroCentering(dps,dataCenter);
        for(int i=0;i<nP;i++){
            lsh.Compute_BCode(dps.d[i] , bCodeData[i]);
        }
        Undo_ZeroCentering(dps,dataCenter);
	} else {
        for(int i=0;i<nP;i++){
            sh.Compute_BCode(dps.d[i] , bCodeData[i]);
        }
    }

}

std::pair<vector<float>,vector<float> > SphericalHashIndexer::knnSearchId(const cv::Mat query, const int n){
    vector<float> indicesFloat;
    vector<float> dists;

    Points qps;

    matToPoints(query,qps);

    bitset<MAX_BCODE_LEN> *bCodeDataQ = new bitset<MAX_BCODE_LEN> [nP];

    int nQ = query.rows;

    if (isLSH){
        Do_ZeroCentering(qps,dataCenter);
        for(int i=0;i<nQ;i++){
            lsh.Compute_BCode( qps.d[i] , bCodeDataQ[i] );
        }
        Undo_ZeroCentering(qps,dataCenter);
	} else {
        for(int i=0;i<nQ;i++){
            sh.Compute_BCode(qps.d[i] , bCodeDataQ[i]);
        }
    }

    std::vector<pair<float,float> > H(nP);

   	Result_Element<int> *res = new Result_Element<int> [ nP ];

    for(int i=0;i<nP;i++){
        float dist = lsh.Compute_HD( bCodeDataQ[0] , bCodeData[i]);
        H.at(i) = std::make_pair (i,dist);
    }

	std::sort(H.begin(),H.end(),compareVectDists());

	for (int i = 0; i < n; i++){
		pair<float,float> p = H.at(i);
		indicesFloat.push_back(p.first);
		dists.push_back(p.second);
	}

    return make_pair(indicesFloat,dists);
}


std::pair<vector<float>,vector<float> > SphericalHashIndexer::radiusSearchId(const cv::Mat query, const double radius, const int n){
    vector<float> indicesFloat;
    vector<float> dists;

    return make_pair(indicesFloat,dists);
}

bool SphericalHashIndexer::save(const string basePath){
    return true;
}

bool SphericalHashIndexer::load(const string basePath){
    return true;
}

string SphericalHashIndexer::getName(){
    return typeId;
}


void SphericalHashIndexer::matToPoints(const cv::Mat& input, Points& output){

    output.Initialize(input.rows,input.cols);

    for (int i = 0; i < input.rows; i++)
        for (int j = 0; j < input.cols; j++)
            output.d[i][j] = (float)( input.at<float>(i,j) );
}


void SphericalHashIndexer::Do_ZeroCentering(Points& in, float* center){
	for(int i=0;i<nP;i++){
		Sub_Vector<REAL_TYPE>( in.d[i] , center , in.d[i] , in.dim );
	}
}

void SphericalHashIndexer::Undo_ZeroCentering(Points& in, float* center){
	for(int i=0;i<nP;i++){
		Add_Vector<REAL_TYPE>( in.d[i] , center , in.d[i] , in.dim );
	}
}

