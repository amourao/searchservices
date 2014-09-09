#include "FlannkNNIndexer.h"

//http://docs.opencv.org/2.4.3/modules/flann/doc/flann_fast_approximate_nearest_neighbor_search.html

static FlannkNNIndexer linearkNNIndexerFactory;

FlannkNNIndexer::FlannkNNIndexer(){
	FactoryIndexer::getInstance()->registerType("flannkNNIndexer",this);
}

FlannkNNIndexer::FlannkNNIndexer(string& typeId){
    type = typeId;
}

FlannkNNIndexer::FlannkNNIndexer(string& typeId, map<string,string>& params){

    type = typeId;

	paramsB = params;
    //FLANN_DIST_EUCLIDEAN = 1,
    //FLANN_DIST_L2 = 1,
    //FLANN_DIST_MANHATTAN = 2,
    //FLANN_DIST_L1 = 2,
    //FLANN_DIST_MINKOWSKI = 3,
    //FLANN_DIST_MAX   = 4,
    //FLANN_DIST_HIST_INTERSECT   = 5,
    //FLANN_DIST_HELLINGER = 6,
    //FLANN_DIST_CHI_SQUARE = 7,
    //FLANN_DIST_CS         = 7,
    //FLANN_DIST_KULLBACK_LEIBLER  = 8,
    //FLANN_DIST_KL                = 8,
    //FLANN_DIST_HAMMING          = 9,

	if(params["distance"] == "EUCLIDEAN")
		flannDistance = cvflann::FLANN_DIST_EUCLIDEAN;
	else if(params["distance"] == "MANHATTAN")
		flannDistance = cvflann::FLANN_DIST_MANHATTAN;
	else if(params["distance"] == "MINKOWSKI")
		flannDistance = cvflann::FLANN_DIST_MINKOWSKI;
	else if(params["distance"] == "HIST_INTERSECT")
		flannDistance = cvflann::FLANN_DIST_HIST_INTERSECT;
	else if(params["distance"] == "HELLINGER")
		flannDistance = cvflann::FLANN_DIST_HELLINGER;
	else if(params["distance"] == "CHI_SQUARE")
		flannDistance = cvflann::FLANN_DIST_CHI_SQUARE;
	else if(params["distance"] == "KULLBACK_LEIBLER")
		flannDistance = cvflann::FLANN_DIST_KULLBACK_LEIBLER;
	else if(params["distance"] == "HAMMING")
		flannDistance = cvflann::FLANN_DIST_HAMMING;

	if(params["algorithm"] == "linear"){
		flannParams = new flann::LinearIndexParams();
	} else if(params["algorithm"] == "kd"){
		flannParams = new flann::KDTreeIndexParams(atoi(params["trees"].c_str()));
		//trees The number of parallel kd-trees to use. Good values are in the range [1..16]
	} else if(params["algorithm"] == "kmeans"){
		cvflann::flann_centers_init_t centers_init;

		if(params["centers_init"] == "CENTERS_RANDOM")
			centers_init = cvflann::CENTERS_RANDOM;
		else if(params["centers_init"] == "CENTERS_GONZALES")
			centers_init = cvflann::CENTERS_GONZALES;
		else //if(params["centers_init"] == "CENTERS_KMEANSPP")
			centers_init = cvflann::CENTERS_KMEANSPP;

		//branching The branching factor to use for the hierarchical k-means tree (32)
		//iterations The maximum number of iterations to use in the k-means clustering stage when building the k-means tree.
		//A value of -1 used here means that the k-means clustering should be iterated until convergence (11)
		//centers_init The algorithm to use for selecting the initial centers when performing a k-means clustering step. (CENTERS_RANDOM)
		//cb_index This parameter (cluster boundary index) influences the way exploration is performed in the hierarchical kmeans tree. When cb_index is zero the next kmeans domain to be explored is chosen to be the one with the closest center. (0.2)
		flannParams = new flann::KMeansIndexParams(atoi(params["branching"].c_str()),
			atoi(params["iterations"].c_str()),
			centers_init,
			atof(params["cb_index"].c_str()));

	} else if(params["algorithm"] == "composite"){
		cvflann::flann_centers_init_t centers_init;

		if(params["centers_init"] == "CENTERS_RANDOM")
			centers_init = cvflann::CENTERS_RANDOM;
		else if(params["centers_init"] == "CENTERS_GONZALES")
			centers_init = cvflann::CENTERS_GONZALES;
		else //if(params["centers_init"] == "CENTERS_KMEANSPP")
			centers_init = cvflann::CENTERS_KMEANSPP;

		flannParams = new flann::CompositeIndexParams(atoi(params["trees"].c_str()),
			atoi(params["branching"].c_str()),
			atoi(params["iterations"].c_str()),
			centers_init,
			atof(params["cb_index"].c_str()));

	} else if(params["algorithm"] == "lsh"){

		//table_number the number of hash tables to use [10...30]
        //key_size the size of the hash key in bits [10...20]
        //multi_probe_level the number of bits to shift to check for neighboring buckets
        //(0 is regular LSH, 2 is recommended).
		flannParams = new flann::LshIndexParams(
        	atoi(params["table_number"].c_str()),
        	atoi(params["key_size"].c_str()),
        	atoi(params["multi_probe_level"].c_str())
        	);
		flannDistance = cvflann::FLANN_DIST_HAMMING;

	} else if(params["algorithm"] == "auto"){

		//target_precision = 0.9, number between 0 and 1 specifying the percentage of the approximate nearest-neighbor searches that return the exact nearest-neighbor.
        //build_weight = 0.01, Specifies the importance of the index build time raported to the nearest-neighbor search time.
        //memory_weight = 0, Is used to specify the tradeoff between time (index build time and search time) and memory used by the index. A value less than 1 gives more importance to the time spent and a value greater than 1 gives more importance to the memory usage.
        //sample_fraction = 0.1, Is a number between 0 and 1 indicating what fraction of the dataset to use in the automatic parameter configuration algorithm. Running the algorithm on the full dataset gives the most accurate results
		flannParams = new flann::AutotunedIndexParams(
        	atof(params["target_precision"].c_str()),
        	atof(params["build_weight"].c_str()),
        	atof(params["memory_weight"].c_str()),
        	atof(params["sample_fraction"].c_str())
        	);
	}
}

FlannkNNIndexer::~FlannkNNIndexer(){

}

void* FlannkNNIndexer::createType(string &typeId, map<string,string>& params){
    return new FlannkNNIndexer(typeId,params);
}

void* FlannkNNIndexer::createType(string &typeId){
	if (typeId == "flannkNNIndexer"){
		return new FlannkNNIndexer(typeId);
	}
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}

void FlannkNNIndexer::train(cv::Mat& featuresTrain,cv::Mat& featuresValidationI,cv::Mat& featuresValidationQ){

}

void FlannkNNIndexer::indexWithTrainedParams(cv::Mat& features){
	indexData = features;

	flannIndexs = new flann::Index(indexData,*flannParams,flannDistance);
}

void FlannkNNIndexer::index(cv::Mat& features){
	//flannIndex = new flann::Index();

	indexData = features;

	flannIndexs = new flann::Index(indexData,*flannParams,flannDistance);
	//flannIndex->build(indexData,flannParams);
}

std::pair<vector<float>,vector<float> > FlannkNNIndexer::knnSearchId(cv::Mat& query, int n){
	vector<int> indices (n);
	vector<float> dists (n);
	//cout << j++ << endl;

	flannIndexs->knnSearch(query,indices,dists,n);

	std::vector<float> indicesFloat(indices.begin(), indices.end());
	return make_pair(indicesFloat,dists);
}

std::pair<vector<float>,vector<float> > FlannkNNIndexer::radiusSearchId(cv::Mat& query, double radius, int n){
	vector<int> indices (n);
	vector<float> dists (n);
	//cout << j++ << endl;

	flannIndexs->radiusSearch(query,indices,dists,radius,n);

	std::vector<float> indicesFloat(indices.begin(), indices.end());
	return make_pair(indicesFloat,dists);
}

bool FlannkNNIndexer::save(string basePath){
	saveLabels(basePath);

	stringstream ss;
	ss << INDEXER_BASE_SAVE_PATH << basePath << INDEX_DATA_EXTENSION_KNN;
	string path = ss.str();
	Mat lab;
	MatrixTools::writeBinV2(path,indexData,lab);

	stringstream ssF;
	ssF << INDEXER_BASE_SAVE_PATH << basePath << INDEX_FLANN_EXTENSION_KNN;

	flannIndexs->save(ssF.str());

	return true;
}

bool FlannkNNIndexer::load(string basePath){
	loadLabels(basePath);

	stringstream ss;
	ss << INDEXER_BASE_SAVE_PATH << basePath << INDEX_DATA_EXTENSION_KNN;
	string path = ss.str();
	Mat lab;
	MatrixTools::readBinV2(path,indexData,indexToLabels);
	//if ( flannIndex != NULL)
	//	delete flannIndex;

	stringstream ssF;
	ssF << INDEXER_BASE_SAVE_PATH << basePath << INDEX_FLANN_EXTENSION_KNN;

	flannParams = new flann::SavedIndexParams(ssF.str());

	flannIndexs = new flann::Index(indexData,*flannParams);

	//flannIndexs->build(indexData,params);
	return true;
}

string FlannkNNIndexer::getName(){
	return type;
}
