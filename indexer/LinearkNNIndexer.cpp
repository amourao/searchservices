#include "LinearkNNIndexer.h"

static LinearkNNIndexer linearkNNIndexerFactory;

LinearkNNIndexer::LinearkNNIndexer(){
	FactoryIndexer::getInstance()->registerType("medicalRetrieval",this);
}

LinearkNNIndexer::~LinearkNNIndexer(){

}

void* LinearkNNIndexer::createType(string &typeId){
	if (typeId == "medicalRetrieval"){
		LinearkNNIndexer* index = new LinearkNNIndexer();
		index->load("");
		return index;
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void LinearkNNIndexer::index(cv::Mat features){
	flann::LinearIndexParams params = flann::LinearIndexParams();

	flannIndex = new flann::Index();
	
	indexData = features;
	flannIndex->build(indexData,params);
}

vector<std::pair<float,float> > LinearkNNIndexer::knnSearchId(cv::Mat query, int n){
	vector<int> indices (n);
	vector<float> dists (n);
	//cout << j++ << endl;

	flannIndex->knnSearch(query,indices,dists,n);

	std::vector<float> indicesFloat(indices.begin(), indices.end());
	return mergeVectors(indicesFloat,dists);
}

vector<std::pair<string,float> > LinearkNNIndexer::knnSearchName(cv::Mat query, int n){
	vector<int> indices (n);
	vector<float> dists (n);
	//cout << j++ << endl;

	flannIndex->knnSearch(query,indices,dists,n);

	std::vector<float> indicesFloat(indices.begin(), indices.end());
	return mergeVectors(idToLabels(indicesFloat),dists);
}

vector<std::pair<float,float> > LinearkNNIndexer::radiusSearchId(cv::Mat query, double radius, int n){
	vector<int> indices (n);
	vector<float> dists (n);
	//cout << j++ << endl;

	flannIndex->radiusSearch(query,indices,dists,radius,n);

	std::vector<float> indicesFloat(indices.begin(), indices.end());
	return mergeVectors(indicesFloat,dists);
}

vector<std::pair<string,float> > LinearkNNIndexer::radiusSearchName(cv::Mat query, double radius, int n){
	vector<int> indices (n);
	vector<float> dists (n);
	//cout << j++ << endl;

	flannIndex->radiusSearch(query,indices,dists,radius,n);

	std::vector<float> indicesFloat(indices.begin(), indices.end());
	return mergeVectors(idToLabels(indicesFloat),dists);
}

bool LinearkNNIndexer::save(string basePath){
	stringstream ss;
	ss << INDEXER_BASE_SAVE_PATH << basePath << INDEX_DATA_EXTENSION_KNN;

	FileStorage fs(ss.str().c_str(), FileStorage::WRITE);

	fs << "indexData" << indexData;
	fs.release();

	stringstream ssL;
	ssL << INDEXER_BASE_SAVE_PATH << basePath << INDEXER_LABELS_EXTENSION;

	saveLabels(ssL.str());

	return true;
}

bool LinearkNNIndexer::load(string basePath){
	
	stringstream ss;
	ss << INDEXER_BASE_SAVE_PATH << basePath << INDEX_DATA_EXTENSION_KNN;

	FileStorage fs(ss.str().c_str(), FileStorage::READ);

	fs["indexData"] >> indexData;

	flann::LinearIndexParams params = flann::LinearIndexParams();

	if ( flannIndex != NULL)
		delete flannIndex;
	flannIndex = new flann::Index();
	
	flannIndex->build(indexData,params);
	
	stringstream ssL;
	ssL << INDEXER_BASE_SAVE_PATH << basePath << INDEXER_LABELS_EXTENSION;

	loadLabels(ssL.str());

	return true;
}

string LinearkNNIndexer::getName(){
	return "LinearKNNIndex";
}
