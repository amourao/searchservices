#include "TextFileSourceV2.h"


TextFileSourceV2::TextFileSourceV2(string _filename){
	readFile(_filename);
	//baseDir = "./ck+/"; //TODO
	imageIndex = 0;
}

TextFileSourceV2::~TextFileSourceV2(){

}

void TextFileSourceV2::readFile(string trainDataFile){
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

		cout << labelFields << " " << indexIdFields << " " << classIdFields<< " " << baseDir << endl;
		while (getline(file, line)) {
			stringstream liness(line);

			getline(liness, path, ';');
			
			stringstream ss2;
			
			ss2 << baseDir << path;
			
			string fullPath = ss2.str();

			cout << baseDir << " " << path << endl;
			imagesPath.push_back(fullPath );
			imagesOriginalInfo.push_back(line);
		}
	}

Mat TextFileSourceV2::nextImage(){
	//cout << imageIndex << " " << !imagesPath.empty() << " " << imagesPath.size() << endl;
	if (!imagesPath.empty() && (unsigned int) imageIndex < imagesPath.size()) {
		string s= imagesPath.at(imageIndex);
		imageIndex++;
		Mat frame2 = imread(s,1);
		return frame2;
	} else 
		return Mat();

}

string TextFileSourceV2::getImageInfo(){
	return imagesOriginalInfo.at(imageIndex-1);
}

string TextFileSourceV2::getImagePath(){
	return imagesPath.at(imageIndex-1);
}

vector<string> TextFileSourceV2::getCurrentImageInfoVector(){
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

string TextFileSourceV2::getCurrentImageInfoField(int field){
	string info = imagesOriginalInfo.at(imageIndex-1);

	stringstream liness(info);

	string id;
	for (int i = 0; i < labelFields-1; i++){
		getline(liness, id, ';');
		if(field == i)
			return id;
	}

	getline(liness, id);
	return id;
}

int TextFileSourceV2::getImageInfoFieldCount(){
	return labelFields;
}

int TextFileSourceV2::getClassFieldId(){
	return classIdFields;
}

int TextFileSourceV2::getIndexFieldId(){
	return indexIdFields;
}

bool TextFileSourceV2::isAvailable(){
	if (!imagesPath.empty() && (unsigned int) imageIndex < imagesPath.size())
		return imread(imagesPath.at(imageIndex),1).empty();
	return false;
}

int TextFileSourceV2::getImageCount(){
	return imagesPath.size();
}

int TextFileSourceV2::getRemainingImageCount(){
	return imagesPath.size() - imageIndex;
}
