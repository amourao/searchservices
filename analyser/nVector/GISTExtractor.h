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

class GISTExtractor :
	public FeatureExtractor
{
public:

	GISTExtractor();
	GISTExtractor(int _nblocks, int _nscale, vector<int> _orientations_per_scale, int _imageW, int _imageH);
	~GISTExtractor();

	void extractFeatures(Mat& src, Mat& dst);
	
	int getFeatureVectorSize();

	string getName();
	
	void* createType(string &typeId);
	
private:

	color_image_t * getGISTColorImage(Mat& src);

    int imageW;
    int imageH;

	int descsize;


	int nblocks;
  	int nscale;
  	int* orientations_per_scale;

	
};

