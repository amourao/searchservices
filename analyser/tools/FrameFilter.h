#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "../sources/TextFileSourceV2.h"

using namespace cv;
using namespace std;

class FrameFilter
{
    public:
        FrameFilter();
        ~FrameFilter();

        static bool hasEdges(const cv::Mat& frame, double thresholdCanny = 85, double thresholdAvg = 1.5);
        static bool hasColor(const cv::Mat& frame, double threshold);

        static bool filter(const cv::Mat& frame);

        static void CannyThreshold(int, void *);
        static int maine(int argc, char** argv);

    private:

};
