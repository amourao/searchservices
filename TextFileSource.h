
#pragma once
#include "ImageSource.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace cv;
using namespace std;

class TextFileSource :
	public ImageSource{

public:
	TextFileSource(string filename);
	~TextFileSource();

	Mat nextImage();
	string getImageInfo();
	bool isAvailable();
	int getImageCount();
	int getRemainingImageCount();
private:

	void readFile(string s);
	int imageIndex;
	string baseDir;
	vector<string> imagesPath;
	vector<string> imagesOriginalInfo;
};


