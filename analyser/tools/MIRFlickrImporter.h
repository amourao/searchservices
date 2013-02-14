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

	void readBin(std::string& file, arma::fmat & features);
	void readTags(std::string& file, arma::uvec& features);



	arma::fmat features;

};
