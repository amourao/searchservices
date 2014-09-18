
#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace cv;
using namespace std;

class VideoTextFileSource {

public:
	VideoTextFileSource(string filename);
	~VideoTextFileSource();

	VideoCapture nextImage();
	string getImageInfo();
	bool isAvailable();
	int getImageCount();
	int getRemainingImageCount();

	void skipTo(int imageIndex);

	vector<string> getCurrentImageInfoVector();
	string getCurrentImageInfoField(int field);
	int getImageInfoFieldCount();
	int getClassFieldId();
	int getIndexFieldId();

	string getImagePath();
	string getBasePath();
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


