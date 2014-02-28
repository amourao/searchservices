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

    SHD = false;
    isLSH = false;


    bCodeLen = atoi(paramsB["bCodeLen"].c_str()); //[64,512]
	trainSamplesCount = atoi(paramsB["trainSamplesCount"].c_str()); //1000
	examineRatio = atof(paramsB["examineRatio"].c_str()); //[0.1;1]


    if (paramsB["algorithm"] == "lsh"){
        isLSH = true;
    } else if(paramsB["algorithm"] == "sh"){
        incRatio = atof(paramsB["incRatio"].c_str());//0.5;
        overRatio = atof(paramsB["overRatio"].c_str());//0.25;
        epsMean = atof(paramsB["epsMean"].c_str());//0.1;
        epsStdDev = atof(paramsB["epsStdDev"].c_str());//0.15;
        maxItr = atof(paramsB["maxItr"].c_str());//50;s

        if (paramsB["distance"] == "shd")
            SHD = true;
        else if (paramsB["distance"] != "hd"){
            cerr << "Invalid distance: " << paramsB["distance"] << endl;
            return;
        }

    } else {
        cerr << "Invalid algorithm: " << paramsB["algorithm"] << endl;
        return;
    }

}

SphericalHashIndexer::SphericalHashIndexer(const string& type){
    typeId = type;

}


SphericalHashIndexer::~SphericalHashIndexer(){
    //delete bCodeData;
    //delete [] dataCenter;
}

void* SphericalHashIndexer::createType(string &type, map<string,string>& params){
    if (!params.empty())
        return new SphericalHashIndexer(type,params);
    else {
        return new SphericalHashIndexer(type);
    }
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
    dataCenter = new float[ dps.dim ];
	// compute mean position of data points
	

    nP = features.rows;
    dims = features.cols;

    if (isLSH){
        dps.Compute_Center( dataCenter);
        lsh.Initialize(dps.dim,bCodeLen);
	} else {
        sh.Initialize(&dps,dps.dim,bCodeLen,trainSamplesCount,maxItr,incRatio,overRatio,epsMean,epsStdDev);
        sh.Set_Spheres();
    }

    //bCodeData = new boost::dynamic_bitset<> [nP];
    bCodeData = new boost::dynamic_bitset<>[nP];

    for(int i=0;i<nP;i++){
        bCodeData[i] = boost::dynamic_bitset<>(MAX_BCODE_LEN);
    }


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

    int nQ = 1;

    boost::dynamic_bitset<> *bCodeDataQ = new boost::dynamic_bitset<> [nQ];

    for(int i=0;i<nQ;i++){
        bCodeDataQ[i] = boost::dynamic_bitset<>(MAX_BCODE_LEN);
    }

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

    for(int i=0;i<nP*examineRatio;i++){
        float dist;

        if(SHD){
            dist = lsh.Compute_SHD( bCodeDataQ[0] , bCodeData[i]);
        } else {
            dist = lsh.Compute_HD( bCodeDataQ[0] , bCodeData[i]);

        }
        H.at(i) = std::make_pair (i,dist);
    }

	std::sort(H.begin(),H.end(),compareVectDists());
	for (int i = 0; i < n; i++){
		pair<float,float> p = H.at(i);
		indicesFloat.push_back(p.first);
		dists.push_back(p.second);
	}

    //delete bCodeDataQ;
    //delete res;
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
	for(int i=0;i<in.nP;i++){
		Sub_Vector<float>( in.d[i] , center , in.d[i] , in.dim );
	}
}

void SphericalHashIndexer::Undo_ZeroCentering(Points& in, float* center){
	for(int i=0;i<in.nP;i++){
		Add_Vector<float>( in.d[i] , center , in.d[i] , in.dim );
	}
}

