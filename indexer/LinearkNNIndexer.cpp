#include "LinearkNNIndexer.h"

static LinearkNNIndexer linearkNNIndexerFactory;

LinearkNNIndexer::LinearkNNIndexer(){
	FactoryIndexer::getInstance()->registerType("MedicalRetrieval",this);
}

LinearkNNIndexer::~LinearkNNIndexer(){

}

void* LinearkNNIndexer::createType(string &typeId){
	if (typeId == "MedicalRetrieval"){
		LinearkNNIndexer* index = new LinearkNNIndexer();
		index->load("");
		return index;
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void LinearkNNIndexer::index(cv::Mat features){

}

vector<float> LinearkNNIndexer::knnSearchId(string name, int n){
	vector<float> v;
	return v;
}

vector<string> LinearkNNIndexer::knnSearchName(string name, int n){
	vector<string> v;
	return v;
}

vector<float> LinearkNNIndexer::radiusSearchId(string name, double radius){
	vector<float> v;
	return v;
}

vector<string> LinearkNNIndexer::radiusSearchName(string name, double radius){
	vector<string> v;
	return v;
}

bool LinearkNNIndexer::save(string basePath){
	return false;
}

bool LinearkNNIndexer::load(string basePath){
	return false;
}

string LinearkNNIndexer::getName(){
	return "LinearKNNIndex";
}
