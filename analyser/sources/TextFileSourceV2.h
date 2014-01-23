
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

class TextFileSourceV2 :
	public ImageSource{

public:
	TextFileSourceV2(string filename);
	~TextFileSourceV2();

	Mat nextImage();
	string getImageInfo();
	bool isAvailable();
	int getImageCount();
	int getRemainingImageCount();

	vector<string> getCurrentImageInfoVector();
	string getCurrentImageInfoField(int field);
	int getImageInfoFieldCount();
	int getClassFieldId();
	int getIndexFieldId();

	string getImagePath();
private:

	void readFile(string s);
	int imageIndex;
	int labelFields;
	int indexIdFields;
	int classIdFields;
	string baseDir;

	vector<string> imagesPath;
	vector<string> imagesOriginalInfo;
};


