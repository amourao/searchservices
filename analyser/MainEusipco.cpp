#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <time.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "nVector/GaborExtractor.h"
#include "nVector/LBPExtractor.h"
#include "nVector/FeatureExtractor.h"

#include "nRoi/FaceDetection.h"

using namespace std;

int getGaborImage(int argc, char *argv[]) {

	string imageNeutralPath(argv[0]);
	string imageExpressionPath(argv[1]);
	string features(argv[2]);

	ImageFilter* filter;

	if(features == 'LBP')
		filter = new LBPExtractor();
	else if(features == 'LBP')
		filter = new GaborExtractor();

	Mat imageNeutral,imageExpression,featuresNeutral,featuresExpression,diff,diffNormalized;

	imread(imageNeutralPath,imageNeutral);
	imread(imageExpressionPath,imageExpression);

	filter->applyFilter(imageNeutral,featuresNeutral);
	filter->applyFilter(imageExpression,featuresExpression);

	diff = featuresExpression-featuresNeutral;

	Mat results;

	normalize(diff, diffNormalized, 0, 255, NORM_MINMAX, CV_8UC1);

	imshow("neutral",featuresNeutral);
	imshow("expression",featuresExpression);
	imshow("diff",diffNormalized);

	waitKey(0);

	delete filter;
	return 0;
}




int main(int argc, char *argv[]){
    return getGaborImage(argc, argv);
}

