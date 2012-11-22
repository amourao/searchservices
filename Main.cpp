#include <iostream>
#include <string>
#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "ImageSource.h"
#include "CamSource.h"
#include "SingleImageSource.h"
#include "VideoSource.h"
#include "FaceDetection.h"

using namespace std;
using namespace cv;


int main(int argc, char *argv[]){


	SingleImageSource vs("./data/teste/faces.png");

	CamSource cs;


	FaceDetection fd("./data/FaceDetection/haarcascades/haarcascade_frontalface_alt.xml","./data/FaceDetection/haarcascades/haarcascade_eye_tree_eyeglasses.xml",4,1, cv::Size(20,20),cv::Size(50,50));

	vector<Mat> faces;
	vector<cv::Point> centers;
	vector<Rect> rois;


	while(cs.isAvailable()){
		Mat image = cs.nextImage();

		double t = (double)cvGetTickCount();
		fd.detectFaces(image,faces,centers,rois);
		t = (double)(cvGetTickCount()-t)/((double)cvGetTickFrequency());
		cout << t/1000.0 << " ms " << faces.size() << " faces" << endl;

		faces.clear();
		centers.clear();
		rois.clear();
	}

	getchar();
	return 0;
}