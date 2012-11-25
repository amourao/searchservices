#include "TextFileSource.h"


TextFileSource::TextFileSource(string _filename){
	readFile(_filename);
	baseDir = "./ck+/"; //TODO
	imageIndex = 0;
}


TextFileSource::~TextFileSource(){

}

void TextFileSource::readFile(string trainDataFile){
	ifstream file(trainDataFile.c_str(), ifstream::in);
		string line, path, numOfSamplesStr, emotionStr;

		getline(file, line);
		stringstream liness(line);
		getline(liness, numOfSamplesStr);	
		path.erase(remove(path.begin(), path.end(), '\r'), path.end());
		path.erase(remove(path.begin(), path.end(), '\n'), path.end());
		path.erase(remove(path.begin(), path.end(), ' '), path.end());

		int i = 0;
		while (getline(file, line)) {
			stringstream liness(line);
			getline(liness, path, ';');
			getline(liness, emotionStr, '\r');
			imagesPath.push_back(path );
			imagesOriginalInfo.push_back(emotionStr);
		}
	}

Mat TextFileSource::nextImage(){
	//cout << imageIndex << " " << !imagesPath.empty() << " " << imagesPath.size() << endl;
	if (!imagesPath.empty() && imageIndex < imagesPath.size()) {
		stringstream fullPath;
		fullPath << baseDir << imagesPath.at(imageIndex);
		Mat frame = imread(fullPath.str(), 1 );
		imagesPath.at(imageIndex++);
		return frame;
	}
	return Mat();

}

string TextFileSource::getImageInfo(){
	return imagesOriginalInfo.at(imageIndex-1);
}

bool TextFileSource::isAvailable(){
	return !imagesPath.empty() && imageIndex < imagesPath.size();
}
