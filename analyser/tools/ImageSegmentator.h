#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

class ImageSegmentator {

public:

    ImageSegmentator();
    ~ImageSegmentator();
    
   
    Mat getStdImage(Mat image);
    vector<int> getCuts(Mat image);
    cv::Rect getRelevantROI(Mat image);
    
    Mat mat2gray(const Mat& src);
    int getRelevantROISingleSide(Mat image);

};
