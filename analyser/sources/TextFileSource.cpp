#include "TextFileSource.h"


TextFileSource::TextFileSource(string _filename){
	readFile(_filename);
	//baseDir = "./ck+/"; //TODO
	baseDir = "";
	imageIndex = 0;
}

TextFileSource::TextFileSource(string filename, string imageBasePath){
	baseDir = imageBasePath;
	readFile(filename);
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
			
			stringstream ss2;
			
			ss2 << baseDir << "/" << path;
			
			string fullPath = ss2.str();
			
			//cout << fullPath << endl;
			
			imagesPath.push_back(fullPath );
			imagesOriginalInfo.push_back(line);
		}
	}

Mat TextFileSource::nextImage(){
	//cout << imageIndex << " " << !imagesPath.empty() << " " << imagesPath.size() << endl;
	if (!imagesPath.empty() && (unsigned int) imageIndex < imagesPath.size()) {
		string s= imagesPath.at(imageIndex);
		//TODO
		imageIndex++;
		//std::cout << imageIndex << " " << s << std::endl;
		Mat frame2 = imread(s,1);
		//std::cout << s << " OK " << frame2.channels() << " " << frame2.empty() << " " << frame2.rows << " " << frame2.cols << std::endl;
		return frame2;
	} else 
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
