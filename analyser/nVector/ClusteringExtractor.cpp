#include "ClusteringExtractor.h"

static ClusteringExtractor ClusteringExtractorFactory;

ClusteringExtractor::ClusteringExtractor(){
    FactoryAnalyser::getInstance()->registerType("ClusteringExtractor",this);

}

ClusteringExtractor::ClusteringExtractor(string& _type){
    type = _type;
}

ClusteringExtractor::ClusteringExtractor(string& _type, map<string, string>& params){
    type = _type;

    if (params.size() == 0)
        return;
     
}

ClusteringExtractor::init(){

    
    flannParams = new flann::LinearIndexParams();
    flannDistance = cvflann::FLANN_DIST_EUCLIDEAN;
    flannIndexs = new flann::Index(indexData,*flannParams);

}

ClusteringExtractor::~ClusteringExtractor(){

}

void* ClusteringExtractor::createType(string& type){
    if (type == "ClusteringExtractor"){
        return new ClusteringExtractor(type);
    }
    cerr << "Error registering type from constructor (this should never happen)" << endl;
    return NULL;
}


void* ClusteringExtractor::createType(string& type, map<string, string>& params){
    return new ClusteringExtractor(type,params);
}

void ClusteringExtractor::extractFeatures(Mat& src, Mat& dst){

}

int ClusteringExtractor::getFeatureVectorSize(){
    return descsize;
}

string ClusteringExtractor::getName(){
    return type;
}

