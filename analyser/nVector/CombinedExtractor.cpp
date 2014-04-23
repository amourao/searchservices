#include "CombinedExtractor.h"

static CombinedExtractor CombinedExtractorFactory;

CombinedExtractor::CombinedExtractor(){
    FactoryAnalyser::getInstance()->registerType("CombinedExtractor",this);

}

CombinedExtractor::CombinedExtractor(string& _type){
    type = _type;
}

CombinedExtractor::CombinedExtractor(string& _type, map<string, string>& params){
    type = _type;

    if (params.size() == 0)
        return;

    algorithmsType = StringTools::split(params["algorithms"],',');

    descsize = 0;
    for (uint i = 0; i < algorithmsType.size(); i++){
        FeatureExtractor* analyser = (FeatureExtractor*)FactoryAnalyser::getInstance()->createType(algorithmsType.at(i));
        descsize += analyser->getFeatureVectorSize();
        algorithms.push_back(analyser);
    }    
}

CombinedExtractor::~CombinedExtractor(){

}

void* CombinedExtractor::createType(string& type){
    if (type == "combinedExtractor"){
        return new CombinedExtractor(type);
    }
    cerr << "Error registering type from constructor (this should never happen)" << endl;
    return NULL;
}


void* CombinedExtractor::createType(string& type, map<string, string>& params){
    return new CombinedExtractor(type,params);
}

void CombinedExtractor::extractFeatures(Mat& src, Mat& dst){
    for(uint i = 0; i < algorithms.size(); i++){
        FeatureExtractor* f = algorithms.at(i);
        Mat feat;
        f->extractFeatures(src,feat);
        if (dst.empty())
            feat.copyTo(dst);
        else
            hconcat(dst,feat,dst);
    }
}

int CombinedExtractor::getFeatureVectorSize(){
    return descsize;
}

string CombinedExtractor::getName(){
    return type;
}

