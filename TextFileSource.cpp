#include "TextFileSource.h"


TextFileSource::TextFileSource(string _filename){
	readFile(_filename);
	//baseDir = "./ck+/"; //TODO
	baseDir = "";
	imageIndex = 0;
}


TextFileSource::~TextFileSource(){

}

void TextFileSource::readFile(string trainDataFile){
	ifstream file(trainDataFile.c_str(), ifstream::in);
		string line, path, numOfSamplesStr, emotionStr,idStr, idStr2,dontCare1,neutralPath;

		getline(file, line);
		stringstream liness(line);
		getline(liness, numOfSamplesStr);	
		path.erase(remove(path.begin(), path.end(), '\r'), path.end());
		path.erase(remove(path.begin(), path.end(), '\n'), path.end());
		path.erase(remove(path.begin(), path.end(), ' '), path.end());

		//int i = 0;
		while (getline(file, line)) {
			//./detectedEmotionCKWithNeutalTrain0.png;s130;1;007;d:\\\\datasets\\\\ck+2\\\\cohn-kanade-images\\\\s130\\\\007\\\\s130_007_00000001.png;./detectedEmotionCKWithNeutalTrain0_neutral.png
			stringstream liness(line);

			getline(liness, path, ';');
			getline(liness, idStr, ';');
			getline(liness, emotionStr, ';');
			getline(liness, idStr2, ';');
			getline(liness, dontCare1, ';');
			getline(liness, neutralPath, '\r');
			
			stringstream ss;
			ss << emotionStr << ";" << neutralPath;
			imagesPath.push_back(path );
			imagesOriginalInfo.push_back(ss.str());
		}
	}

Mat TextFileSource::nextImage(){
	//cout << imageIndex << " " << !imagesPath.empty() << " " << imagesPath.size() << endl;
	if (!imagesPath.empty() && (unsigned int) imageIndex < imagesPath.size()) {
		string s= imagesPath.at(imageIndex);
		#ifdef __linux__
			stringstream fullPath;
			fullPath << baseDir << imagesPath.at(imageIndex);
			s = fullPath.str();
		#endif
		Mat frame = imread(s, 1 );
		imagesPath.at(imageIndex++);
		return frame;
	}
	return Mat();

}

string TextFileSource::getImageInfo(){
	return imagesOriginalInfo.at(imageIndex-1);
}

bool TextFileSource::isAvailable(){
	return !imagesPath.empty() && (unsigned int)  imageIndex < imagesPath.size();
}

	int TextFileSource::getImageCount(){
		return imagesPath.size();
	}
	
	int TextFileSource::getRemainingImageCount(){
		return imagesPath.size() - imageIndex;
	}
