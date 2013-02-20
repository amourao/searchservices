#pragma once

#include <armadillo>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <map>

class MIRFlickrImporter {

public:
	MIRFlickrImporter();
	~MIRFlickrImporter();

	//reads a single binary features file
	void readBin(std::string file, arma::fmat& features);
	void readTags(std::string file, arma::uvec& tags);
	void readAllTags(std::string file);
	void getTagFeatures(std::string tagName, arma::uvec& features);

	void subtractTags(std::string tagName, std::string tagToSubtract, arma::uvec& tags);

	void getTrainData(arma::fmat& features,std::map<std::string,arma::uvec>& tags);
	void getTestData(arma::fmat& features,std::map<std::string,arma::uvec>& tags);




	std::vector<std::vector<std::string> > testTags;
	arma::uvec testDataIndex;
	arma::uvec trainDataIndex;
private:
	arma::fmat features;

	std::map<std::string,arma::uvec> tags;

	std::map<std::string,arma::uvec> tagsTrain;
	std::map<std::string,arma::uvec> tagsTest;



	double ratio;
};
