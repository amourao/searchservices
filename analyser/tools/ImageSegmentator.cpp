#include "ImageSegmentator.h"


ImageSegmentator::ImageSegmentator(){

}

ImageSegmentator::~ImageSegmentator(){

}

Mat ImageSegmentator::mat2gray(const Mat& src)
{
    Mat dst;
    normalize(src, dst, 0.0, 1.0, NORM_MINMAX);
    return dst;
}

Mat ImageSegmentator::getStdImage(Mat image){
    Mat image32f;
    image.convertTo(image32f, CV_32F);

    Mat mu;
    blur(image32f, mu, Size(3, 3));

    Mat mu2;
    blur(image32f.mul(image32f), mu2, Size(3, 3));

    Mat sigma;
    cv::sqrt(mu2 - mu.mul(mu), sigma);

    //imshow("coke", mat2gray(image32f));
    //imshow("mu", mat2gray(mu));
    return mat2gray(sigma);
}

vector<int> ImageSegmentator::getCuts(Mat image2){

    vector<int> results;

    Mat image = image2;


    vector<double> values;
    vector<double> valuesMax;

    double max = 0;
    double min = DBL_MAX;

    double maxQ = 0;
    double minQ = DBL_MAX;


    for (int i = 0; i < image.cols; i++){

        double m, M;
        Point p_min, p_max;


        minMaxLoc(image.col(i), &m, &M, &p_min, &p_max);


        if (m > maxQ)
            maxQ = m;
        if (m < minQ)
            minQ = m;

        valuesMax.push_back(m);
        cv::Scalar mean1, stddev1;
        cv::meanStdDev(image.col(i), mean1, stddev1);
        m = stddev1.val[0];


        /*
        m = 0;
        for (int j = 0; j < image.rows; j++){
            m += image.at<float>(j,i);
        }
        m /= image.rows;
        */

        values.push_back(m);

        if (m > max)
            max = m;
        if (m < min)
            min = m;
    }

    int contours = 0;

    int last = 0;

    bool wasContour = false;

    for (uint i = 0; i < values.size(); i++){

        //if (values.at(i) > max*0.9 && !wasContour && (i-last) > values.size()*0.15){
        if ((((valuesMax.at(i) > (maxQ*0.9)) && (valuesMax.at(i) > 240)) || (values.at(i) <= (min*1.2) && values.at(i) < 2)) && !wasContour && (i-last) > values.size()*0.15 && i < values.size()*(1-0.15) ){
            contours++;
            last = i;
            results.push_back(i);
            wasContour = true;
            //line(imageColor,cv::Point(i,0),cv::Point(i,image.rows),CV_RGB(0,0,255));
        } else {
            wasContour = false;
        }
    }
    //cout << maxQ << " " << min << endl;


    return results;
}

cv::Rect ImageSegmentator::getRelevantROI(Mat image){
    Mat image2,image3,image4;
    int x,y,w,h;
    x = getRelevantROISingleSide(image);

    /*
     * flipCode  Specifies how to flip the array: 0 means flipping around the x-axis, positive (e.g., 1) means flipping around y-axis, and negative (e.g., -1) means flipping around both axes. See also the discussion below for the formulas.
     */
    flip(image, image2, 0);
    w = image.cols - getRelevantROISingleSide(image2) - x;

    cv::transpose(image, image3);
    cv::flip(image3, image3, 1);
    y = getRelevantROISingleSide(image3);

    flip(image3, image4, 0);
    h = image.rows - getRelevantROISingleSide(image4) - y;

    cout << cv::Rect(x,y,w,h) << endl;
    return cv::Rect(x,y,w,h);
}

int ImageSegmentator::getRelevantROISingleSide(Mat image){
     //cv::Rect roi(image2.cols*0.1,image2.rows*0.1,image2.cols*0.8,image2.rows*0.8);
    //Mat image(image2,roi);
    for (int i = 0; i < image.cols-1; i++){
        Mat diff;
        absdiff(image.col(i),image.col(i+1), diff);
         double mean2 = mean(diff).val[0];
         cout << mean2 << endl;
         if(mean2 > 0.5)
            return i;
    }
    return image.cols;
}


