#include "MIRFlickrImporter.h"

MIRFlickrImporter::MIRFlickrImporter() {
	// TODO Auto-generated constructor stub

}

MIRFlickrImporter::~MIRFlickrImporter() {
	// TODO Auto-generated destructor stub
}

void MIRFlickrImporter::readBin(std::string file, arma::fmat& _features) {
	std::string f = file;

	int numSamples = 25000;
	int dimensions = 0;

	std::fstream ifs(f.c_str(), std::ios::in | std::ios::binary);
	ifs.seekg(0, std::ifstream::end);
	int fileSize = ifs.tellg();

	//std::cout << fileSize / (double) numSamples / sizeof(float) << std::endl;

	dimensions = fileSize / numSamples / sizeof(float);

	_features = arma::fmat(numSamples, dimensions);

	ifs.seekg(0, std::ifstream::beg);

	int trainIndex = 0;
	int testIndex = 0;
	trainDataIndex.resize(3*numSamples/5);
	testDataIndex.resize(2*numSamples/5);
	for (int i = 0; i < numSamples; i++) {
		for (int j = 0; j < dimensions; j++) {
			float feature = 0;
			ifs.read((char*) &feature, sizeof(feature));
			_features(i, j) = feature;
		}
		if (i % 5 < 3)
			trainDataIndex[trainIndex++] = i;
		else
			testDataIndex[testIndex++] = i;

	}
	ifs.close();
	features = _features;
}

// reads the tags file (with an imageId per line) into a arma::uvec vector
// also takes into account the division between train and test images
void MIRFlickrImporter::readTags(std::string file, arma::uvec& features) {
	std::fstream backstory (file.c_str(), std::ios::in);
	std::string line;
	int i = 0;

	int trainIndex = 0;
	int testIndex = 0;


	arma::uvec featuresTrain;
	arma::uvec featuresTest;

	unsigned found = file.find_last_of("/\\");
	unsigned foundDot = file.find_last_of(".");
	std::string tagName= file.substr(found+1,foundDot-found-1);

	while (getline(backstory,line)){
		int index = atoi(line.c_str())-1;

	    features.insert_rows(i,1);
	    features.at(i++) = index;

	    int originalIndex;
	    arma::uvec tmp;
		if (index % 5 < 3){
			tmp =  arma::find(trainDataIndex == index);
			originalIndex = tmp.at(0);
			featuresTrain.insert_rows(trainIndex,1);
			featuresTrain.at(trainIndex++) = originalIndex;
		} else {
			tmp =  arma::find(testDataIndex == index);
			originalIndex = tmp.at(0);
			featuresTest.insert_rows(testIndex,1);
			featuresTest.at(testIndex++) = originalIndex;
			if (testTags.size() <= index){
				testTags.resize(index+1);
			}
			testTags.at(index).push_back(tagName);
		}
	}

	//std::cout << tagName << " " << features.size() << " " << featuresTrain.size() << " " << featuresTest.size() << std::endl;
	tags[tagName]=features;
	tagsTrain[tagName]=featuresTrain;
	tagsTest[tagName]=featuresTest;
}

// reads a tag set file files (one tag file per fine)
void MIRFlickrImporter::readAllTags(std::string file) {
	std::fstream backstory (file.c_str(), std::ios::in);
	std::string line;
	arma::uvec features;
	while (getline(backstory,line)){
		readTags(line,features);
		features.clear();
	}
}

void MIRFlickrImporter::subtractTags(std::string tagName,std::string tagToSubtract, arma::uvec& tagsReturn) {
	arma::uvec original = tags[tagName];
	arma::uvec subtract = tags[tagToSubtract];


	int lastIdOfTheTaggedElement = 0;
	int tmpIndex = 0;

	//Removes duplicated tags
	//TODO: files (and tags vector) must be ordered,
	for(unsigned int s = 0; s < original.n_rows; s++){
		if(lastIdOfTheTaggedElement >= subtract.n_rows || original(s) != subtract(lastIdOfTheTaggedElement)){
			tagsReturn.insert_rows(tmpIndex,1);
			tagsReturn.at(tmpIndex++) = original(s);
		} else {
			lastIdOfTheTaggedElement++;
		}
	}
	//tagsReturn = find(original != subtract);
}

void MIRFlickrImporter::getTagFeatures(std::string tagName, arma::uvec& features){
	features = tags[tagName];
}

void MIRFlickrImporter::getTrainData(arma::fmat& _features,
		std::map<std::string, arma::uvec>& _tags) {
	arma::uvec filteredCols(features.n_cols);
	for(unsigned int s = 0; s < features.n_cols; s++)
		filteredCols(s) = s;
	_features = features.submat(trainDataIndex,filteredCols);
	_tags = tagsTrain;
}

void MIRFlickrImporter::getTestData(arma::fmat& _features,
		std::map<std::string, arma::uvec>& _tags) {
	arma::uvec filteredCols(features.n_cols);
	for(unsigned int s = 0; s < features.n_cols; s++)
		filteredCols(s) = s;
	_features = features.submat(testDataIndex,filteredCols);
	_tags = tagsTest;
}




