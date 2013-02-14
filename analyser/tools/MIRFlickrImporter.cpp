#include "MIRFlickrImporter.h"

MIRFlickrImporter::MIRFlickrImporter() {
	// TODO Auto-generated constructor stub

}

MIRFlickrImporter::~MIRFlickrImporter() {
	// TODO Auto-generated destructor stub
}

void MIRFlickrImporter::readBin(std::string& file, arma::fmat& _features) {
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

	for (int i = 0; i < numSamples; i++) {
		for (int j = 0; j < dimensions; j++) {
			float feature = 0;
			ifs.read((char*) &feature, sizeof(feature));
			_features(i, j) = feature;
		}
	}
	ifs.close();

	features = _features;

}

void MIRFlickrImporter::readTags(std::string& file, arma::uvec& features) {
	std::fstream backstory (file.c_str(), std::ios::in);
	std::string line;
	int i = 0;
	while (getline(backstory,line)){
	    features.insert_rows(i,1);
	    features.at(i++) = atoi(line.c_str());
	}
}

