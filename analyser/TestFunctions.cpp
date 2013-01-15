#include <iostream>
#include <string>
#include <sstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


#include <opencv2/contrib/detection_based_tracker.hpp>


#include "sources/ImageSource.h"
#include "sources/CamSource.h"
#include "sources/SingleImageSource.h"
#include "sources/VideoSource.h"
#include "extractors/FaceDetection.h"

using namespace std;
using namespace cv;

int openCVFaceTracker() {

	//LINUX ONLY
	/*
	vector< Rect_<int> > faces;
	DetectionBasedTracker::Parameters param;
    param.maxObjectSize = 400;
    param.maxTrackLifetime = 20;
    param.minDetectionPeriod = 7;
    param.minNeighbors = 3;
    param.minObjectSize = 20;
    param.scaleFactor = 1.1;

	DetectionBasedTracker obj = DetectionBasedTracker("./data/FaceDetection/haarcascades/haarcascade_frontalface_alt.xml",param);

	obj.run();

    VideoCapture cap(0);

    Mat img,gray;
    cv::Rect_<int> face_i;

    cv::namedWindow("Detection Based Tracker",cv::WINDOW_NORMAL);

    cv::setWindowProperty("Detection Based Tracker", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

    for(;;)
    {

        cap>>img;
        cv::cvtColor(img,gray,CV_RGB2GRAY);


        // The class object is run on a gray image.

        obj.process(gray);

        // The results are a vector of Rect that enclose the object being tracked
        obj.getObjects(faces);

       // if(faces.size() == 0) obj.resetTracking();

        for(int i = 0; i < faces.size(); i++)
        {
            face_i = faces[i];

            // Make a rectangle around the detected object

            rectangle(img, face_i, CV_RGB(0, 255,0), 3);

            string box_text = format("Tracked Area");

            int pos_x = std::max(face_i.tl().x - 10, 0);
            int pos_y = std::max(face_i.tl().y - 10, 0);
            // And now put it into the image:
            putText(img, box_text, Point(pos_x, pos_y), FONT_HERSHEY_SIMPLEX, 1.0, CV_RGB(0,255,0), 2.0);

         }

    cv::imshow("Detection Based Tracker",img);      // Show the results.

    if(cv::waitKey(33) == 27) break;

    }

    obj.stop();
	*/
	return 0;

}

int testFaceDetectScales(){


	SingleImageSource vs("./data/faces.png");

	cv::Size minSizes[]={cv::Size(10,10),cv::Size(20,20),cv::Size(30,30),cv::Size(50,50),cv::Size(100,100),cv::Size(200,200)};

	for (int scale = 2; scale <= 10; scale+=2){
		for (int i = 0; i < 6; i++){
			for (int j = i; j < 6; j++){

				cout << "scale: " << scale << " minSize: " << minSizes[i]<< " maxSize: " <<minSizes[j] << endl;
				FaceDetection fd("./data/FaceDetection/haarcascades/haarcascade_frontalface_alt.xml","./data/FaceDetection/haarcascades/haarcascade_eye_tree_eyeglasses.xml",scale,2, minSizes[i],minSizes[j]);

				vector<Mat> faces;
				vector<cv::Point> centers;
				vector<Rect> rois;

				Mat image = vs.nextImage();

				fd.detectFaces(image,faces,centers,rois);
			}
		}
	}
	return 0;
}
