#include "searchDemo.h"
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>
#include <flann/flann.hpp>
#include <flann/io/hdf5.h>
#include <stdlib.h>


static searchDemo searchDemoFactory;

searchDemo::searchDemo(string type)
{
	this->type = type;
}

searchDemo::searchDemo() {
	FactoryEndpoint::getInstance()->registerType("/searchText",this);
	FactoryEndpoint::getInstance()->registerType("/searchSimilarTag",this);
	FactoryEndpoint::getInstance()->registerType("/searchSimilarVisual",this);
	FactoryEndpoint::getInstance()->registerType("/searchSimilarGenre",this);

	Matrix<float> datasetTags;
	Matrix<float> datasetVisual;
	Matrix<float> datasetGenre;

	// read list of image file names
	vector<string> imageFileNames;
	readFileNames("../data/images.txt", &imageFileNames);

	// read image tags
	vector<vector<string>> imageTags;
	//readFileNames("path", &imageTags);

	// read feature matrices
	//TODO How do I know the number of row?
	readBinPoints("../data/feat.gabor-4-6.ann",datasetTags,25000);
	readBinPoints("../data/feat.margHSV_lin-16+16+16-H-G.T3x3.ann", datasetVisual,25000);
	readBinPoints("../data/feat.tamuraS-2-A.T3x3.ann", datasetGenre,25000);

	// NIndexFactory::getInstance->createType("KDTree");
	indexTags(datasetTags, flann::KDTreeIndexParams(4));
	indexVisual(datasetVisual, flann::KDTreeIndexParams(4));
	indexGenre(datasetGenre, flann::KDTreeIndexParams(4));

	indexTags.buildIndex();
	indexVisual.buildIndex();
	indexGenre.buildIndex();
}

void searchDemo::readBinPoints(string fname, Matrix<float> &mat, int nrows)
{
	ifstream f(fname.c_str());

	if (f.is_open()) { // check open

		int ncols = f.seekg(0,ifstream::end).tellg() / nrows /sizeof(float);
		mat = malloc(nrows*ncols*sizeof(float));

		f.seekg(0,ifstream::beg);

		float a;
		for (int i = 0; i!= nrows; i++) {
			for (int j = 0; j!= ncols; j++) {
				f.read((char*)&a, sizeof(float));
				mat[i][j] = a;
			}
		}
	}
}

void searchDemo::readFileNames(string fname, vector<string> *fileNames)
{
	ifstream f(fname.c_str());
	fileNames = new vector<string>();
	string line;
	while(std::getline(f,line) != f.eofbit)
		fileNames->push_back(line);
}

searchDemo::~searchDemo() {
	// TODO Auto-generated destructor stub
}

void* searchDemo::createType(string& type){
	//TODO
	cout << "New type requested: " << type << endl;

	if (type == "/searchText" ||
			type == "/searchSimilarTag" ||
			type == "/searchSimilarVisual" ||
			type == "/searchSimilarGenre")
		return new searchDemo(type);

	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void searchDemo::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse &resp)
{
	if(method != "GET"){
		resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		resp.send();
		return ;
	}

	resp.setContentType("application/json");
	ostream& out = resp.send();

	string response("");

	if (type == "/searchText")
		response = searchText(queryStrings);
	else if(type == "/searchSimilarTag")
		response = searchSimilarTag(queryStrings);
	else if(type == "/searchSimilarVisual")
		response = searchSimilarVisual(queryStrings);
	else if(type == "/searchSimilarGenre")
		response = searchSimilarGenre(queryStrings);

	out << response;
	out.flush();
}


string searchDemo::searchText(map<string, string> parameters)
{
	map<string,string>::iterator it;
	for(it=parameters.begin(); it < parameters.end(); it++)
	{
		cout << it->first << endl;
		cout << it->second << endl;
	}
	Value root;
	Value results;
	for(int i = 0; i < 8; i++)
	{
		Value result;
		Value tags;
		result["id"] = rand() % 25000;
		result["rank"] = rand() % 10;
		result["path"] = "/some/path/";
		result["title"] = "randomTitle";
		tags[0] = "tag1";
		tags[1] = "tag2";
		tags[2] = "tag3";
		result["tags"] = tags;
		results[i] = result;
	}
	root["results"] = results;
	return root.asString();
};

//TODO
string searchDemo::searchSimilarTag(map<string, string> parameters)
{



	// for each query we must return 8 results
	int nn = 8;

	// create a matrix to hold search results for each query vector
	// this will be a matrix with query.rows rows and nn columns
	flann::Matrix<int> indices(new int[query.rows*nn], query.rows, nn);
	flann::Matrix<float> dists(new float[query.rows*nn], query.rows, nn);
	indexTags.knnSearch(query, indices, dists, nn, flann::SearchParams(128));

	flann::Matrix<float> query;
	string ids = parameters["id"];
	while(ids.size() > 0)
	{
		size_t splitPos = ids.find(";");
		string id = ids.substr(0,splitPos);
		ids = ids.substr(splitPos+1,ids.size()-splitPos-1);
	}


}
for each id in parameters
get feature vector (id) from the correspnding SQL blob table
add new row to query with current feature vector



Value root;
Value results;
for(int i = 0; i < 8; i++)
{
	Value result;
	Value tags;
	result["id"] = indices[i];
	result["rank"] = dists[i];
	result["path"] = "/some/path/" + imageFileNames[indices[i]];
	result["title"] = "randomTitle";
	tags[0] = "tag1";
	tags[1] = "tag2";
	tags[2] = "tag3";
	result["tags"] = tags;
	results[i] = result;
}
root["results"] = results;
return root.asString();

};

//TODO
string searchDemo::searchSimilarVisual(map<string, string> parameters)
{
	//	int nn = 8;
	//
	//	flann::Matrix<int> indices(new int[query.rows*nn], query.rows, nn);
	//	flann::Matrix<float> dists(new float[query.rows*nn], query.rows, nn);
	//	indexVisual.knnSearch(query, indices, dists, nn, flann::SearchParams(128));
	//	return "";
};

//TODO
string searchDemo::searchSimilarGenre(map<string, string> parameters)
{
	//	int nn = 8;
	//
	//	flann::Matrix<int> indices(new int[query.rows*nn], query.rows, nn);
	//	flann::Matrix<float> dists(new float[query.rows*nn], query.rows, nn);
	//	indexGenre.knnSearch(query, indices, dists, nn, flann::SearchParams(128));
	//	return "";
};
