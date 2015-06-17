#include "GISTExtractor.h"

static GISTExtractor GISTExtractorFactory;

GISTExtractor::GISTExtractor(){
    FactoryAnalyser::getInstance()->registerType("gistExtractor",this);

}

GISTExtractor::GISTExtractor(string& _type){
    type = _type;
}

GISTExtractor::GISTExtractor(string& _type, map<string, string>& params){
    type = _type;

    if (params.size() == 0)
        return;

    int _nblocks = atoi(params["nBlocks"].c_str());
    int _nscale = atoi(params["nScales"].c_str());
    int _imageW = atoi(params["imageW"].c_str());
    int _imageH = atoi(params["imageH"].c_str());

    vector<int> _orientations_per_scale;

    string opsString = params["ops"];
    vector<string> opsSplit = StringTools::split(opsString,';');
    for(uint i = 0; i < opsSplit.size(); i++){
        _orientations_per_scale.push_back(atoi(opsSplit.at(i).c_str()));
    }

    init( _nblocks, _nscale, _orientations_per_scale, _imageW, _imageH);

}

GISTExtractor::GISTExtractor(int _nblocks, int _nscale, vector<int> _orientations_per_scale, int _imageW, int _imageH){
    init( _nblocks, _nscale, _orientations_per_scale, _imageW, _imageH);
}


GISTExtractor::~GISTExtractor(){

}

void* GISTExtractor::createType(string& type){
    //int nblocks=4;
    //int n_scale=3;
    //int orientations_per_scale[50]={8,8,4};

    //vector<int> ops;
    //ops.push_back(8);
    //ops.push_back(8);
    //ops.push_back(4);
    //return new GISTExtractor(4,3,ops,320,320);

    if (type == "gistExtractor"){
        return new GISTExtractor(type);
    }
    cerr << "Error registering type from constructor (this should never happen)" << endl;
    return NULL;
}


void* GISTExtractor::createType(string& type, map<string, string>& params){
    return new GISTExtractor(type,params);
}

void GISTExtractor::init(int _nblocks, int _nscale, vector<int> _orientations_per_scale, int _imageW, int _imageH){
    imageW = _imageW;
    imageH = _imageH;
    nblocks = _nblocks;
    nscale = _nscale;
    orientations_per_scale = new int[_nscale];

    for(int i = 0; i < _nscale; i++){
        orientations_per_scale[i] = _orientations_per_scale[i];
    }

    descsize=0;
    for(int i=0;i<nscale;i++)
        descsize+=nblocks*nblocks*orientations_per_scale[i];
    descsize*=3;
}

void GISTExtractor::extractFeatures(Mat& src, Mat& dst){

    Mat newSrc;
    resize(src,newSrc,cv::Size(imageW,imageH),0,0,INTER_CUBIC);

    color_image_t * im = getGISTColorImage(newSrc);

    float *desc=color_gist_scaletab(im,nblocks,nscale,orientations_per_scale);
    //color_gist(im, nblocks, 1, 2, 3);

    Mat newDst = Mat(1, descsize, CV_32F);

    for (int i = 0; i < descsize; i++)
        newDst.at<float>(0,i) = desc[i];

    color_image_delete(im);
    delete[] desc;
    newDst.copyTo(dst);
}

int GISTExtractor::getFeatureVectorSize(){
    return descsize;
}

color_image_t * GISTExtractor::getGISTColorImage(Mat& src) {

    Mat newSrc;

    src.copyTo(newSrc);

    if (newSrc.type() == CV_32FC3 || newSrc.type() == CV_32FC4)
        newSrc.convertTo(newSrc,CV_8UC3);
    else if (newSrc.type() == CV_32F)
        newSrc.convertTo(newSrc,CV_8U);


    int width = src.cols;
    int height = src.rows;

    color_image_t *im=color_image_new(width,height);
    int k;
    for(int i=0;i<height; i++){
        for(int j=0;j<width; j++){
            k = i*width+j;
            if (newSrc.channels() == 3){
                im->c1[k]=newSrc.at<Vec3b>(i,j)[0];
                im->c2[k]=newSrc.at<Vec3b>(i,j)[1];
                im->c3[k]=newSrc.at<Vec3b>(i,j)[2];
            } else if (newSrc.channels() == 1){
                im->c1[k]=newSrc.at<int>(i,j);
                im->c2[k]=newSrc.at<int>(i,j);
                im->c3[k]=newSrc.at<int>(i,j);
            }
        }
    }
    return im;
}



string GISTExtractor::getName(){
    return type;
}

