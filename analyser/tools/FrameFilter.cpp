#include "FrameFilter.h"

    Mat src,src_gray;
    Mat dst, detected_edges;
    int edgeThresh = 1;
    int lowThreshold;
    int const max_lowThreshold = 100;
    int ratio = 3;
    int kernel_size = 3;
    char * window_name = "Edge Map";
    char * window_name_2 = "Original Image";

FrameFilter::FrameFilter(){
    //ctor
}

FrameFilter::~FrameFilter(){
    //dtor
}
bool FrameFilter::hasEdges(const Mat& frame, double thresholdCanny, double thresholdAvg){
    cv::Rect myROI(frame.cols*0.1, frame.rows*0.1, frame.cols*0.8, frame.rows*0.8);
    Mat frameRoi = frame(myROI);
    Mat frameRoiGray;
    cvtColor( frameRoi, frameRoiGray, CV_BGR2GRAY );

    Mat edges;
    blur( frameRoiGray, edges, Size(3,3) );
    /// Canny detector
    Canny( edges, edges, thresholdCanny, thresholdCanny*3, 3 );
    /// Using Canny’s output as a mask, we display our result
    Scalar s = mean(edges);
    double value = (s[0] + s[1] + s[2]);
    return value > thresholdAvg;
}

bool FrameFilter::hasColor(const Mat& frame, double threshold){

}

bool FrameFilter::filter(const Mat& frame){

}

void FrameFilter::CannyThreshold(int, void *)
{
    /// Reduce noise with a kernel 3x3
    blur( src_gray, detected_edges, Size(3,3) );
    /// Canny detector
    Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );
    /// Using Canny’s output as a mask, we display our result
    dst = Scalar::all(0);
    src.copyTo( dst, detected_edges);

    Scalar s = mean(dst);
    cout << (s[0] + s[1] + s[2]) << endl;

    imshow( window_name, dst );
    imshow( window_name_2, src );
}

int FrameFilter::maine(int argc, char** argv){

    /// Load an image
    string trainData = string(argv[1]);
	TextFileSourceV2 is(trainData);
	Mat frame;
	int j = 0;
    while (j < is.getImageCount()) {
        frame = is.nextImage();
        cv::Rect myROI(frame.cols*0.1, frame.rows*0.1, frame.cols*0.8, frame.rows*0.8);
        src = frame(myROI);
        if( !src.data )
        { return -1; }
        /// Create a matrix of the same type and size as src (for dst)
        dst.create( src.size(), src.type() );
        /// Convert the image to grayscale
        cvtColor( src, src_gray, CV_BGR2GRAY );
        /// Create a window
        namedWindow( window_name, CV_WINDOW_AUTOSIZE );
        /// Create a Trackbar for user to enter threshold
        createTrackbar( "Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold );
        /// Show the image
        FrameFilter::CannyThreshold(0, 0);
        /// Wait until user exit program by pressing a key
        waitKey(0);
    }
    return 0;
}
