#include "VideoTextFileSource.h"


VideoTextFileSource::VideoTextFileSource(string _filename){
	readFile(_filename);
	imageIndex = 0;
}

VideoTextFileSource::~VideoTextFileSource(){

}

void VideoTextFileSource::readFile(string trainDataFile){
	ifstream file(trainDataFile.c_str(), ifstream::in);
		string line, path, tmpStr, emotionStr,idStr, idStr2,dontCare1,neutralPath;

		getline(file, line);

		getline(file, tmpStr);//number of label fields
		labelFields = atoi(tmpStr.c_str());

		getline(file, tmpStr);// index label id
		indexIdFields = atoi(tmpStr.c_str());
		getline(file, tmpStr);// classification label id
		classIdFields = atoi(tmpStr.c_str());

		getline(file, tmpStr);// base image path

		tmpStr.erase(remove(tmpStr.begin(), tmpStr.end(), '\r'), tmpStr.end());
		tmpStr.erase(remove(tmpStr.begin(), tmpStr.end(), '\n'), tmpStr.end());
		baseDir = tmpStr;
		while (getline(file, line)) {
			stringstream liness(line);

			getline(liness, path, ';');

			stringstream ss2;

			ss2 << baseDir << path;

			string fullPath = ss2.str();

			imagesPath.push_back(fullPath);

			imagesOriginalInfo.push_back(line);
		}
	}

VideoCapture VideoTextFileSource::nextImage(){
	VideoCapture capture;
	//cout << imageIndex << " " << !imagesPath.empty() << " " << imagesPath.size() << endl;
	if (!imagesPath.empty() && (unsigned int) imageIndex < imagesPath.size()) {
		string s= imagesPath.at(imageIndex);
		imageIndex++;
		capture = VideoCapture(s);
	} 
	return capture;

}

void VideoTextFileSource::skipTo(int _imageIndex){
    imageIndex = _imageIndex;
}

string VideoTextFileSource::getImageInfo(){
	return imagesOriginalInfo.at(imageIndex-1);
}

string VideoTextFileSource::getImagePath(){
	return imagesPath.at(imageIndex-1);
}

vector<string> VideoTextFileSource::getCurrentImageInfoVector(){
	string info = imagesOriginalInfo.at(imageIndex-1);

	stringstream liness(info);

	vector<string> result;
	string id;
	for (int i = 0; i < labelFields-1; i++){
		getline(liness, id, ';');
		result.push_back(id);
	}
	getline(liness, id);
	result.push_back(id);
	return result;
}

string VideoTextFileSource::getCurrentImageInfoField(int field){
	string info = imagesOriginalInfo.at(imageIndex-1);

	stringstream liness(info);

	string id;
	for (int i = 0; i < labelFields-1; i++){
		getline(liness, id, ';');
		if(field == i)
			return id;
	}

	getline(liness, id);
	id.erase(remove(id.begin(), id.end(), '\r'), id.end());
	id.erase(remove(id.begin(), id.end(), '\n'), id.end());
	return id;
}

int VideoTextFileSource::getImageInfoFieldCount(){
	return labelFields;
}

int VideoTextFileSource::getClassFieldId(){
	return classIdFields;
}

int VideoTextFileSource::getIndexFieldId(){
	return indexIdFields;
}

bool VideoTextFileSource::isAvailable(){
	if (!imagesPath.empty() && (unsigned int) imageIndex < imagesPath.size())
		return imread(imagesPath.at(imageIndex),1).empty();
	return false;
}

int VideoTextFileSource::getImageCount(){
	return imagesPath.size();
}

int VideoTextFileSource::getRemainingImageCount(){
	return imagesPath.size() - imageIndex;
}

string VideoTextFileSource::getBasePath(){
	return baseDir;
}