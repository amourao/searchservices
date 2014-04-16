#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "standalone_image.h"
#include "gist.h"

#ifdef __cplusplus
}
#endif


#include "FeatureExtractor.h"
#include "../FactoryAnalyser.h"
#include "../../commons/StringTools.h"

class GISTExtractor :
	public FeatureExtractor
{
public:

	GISTExtractor();
	GISTExtractor(string& type);
	GISTExtractor(string& type, map<string, string>& params);

	GISTExtractor(int _nblocks, int _nscale, vector<int> _orientations_per_scale, int _imageW, int _imageH);
	~GISTExtractor();

    void init(int _nblocks, int _nscale, vector<int> _orientations_per_scale, int _imageW, int _imageH);

    void* createType(string &typeId);
	void* createType(string &typeId, map<string, string>& params);

	void extractFeatures(Mat& src, Mat& dst);

	int getFeatureVectorSize();

	string getName();



private:
    string type;

	color_image_t * getGISTColorImage(Mat& src);

    int imageW;
    int imageH;

	int descsize;


	int nblocks;
  	int nscale;
  	int* orientations_per_scale;


};

