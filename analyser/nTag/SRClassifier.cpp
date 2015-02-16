#include "SRClassifier.h"

static SRClassifier SRClassifierFactory;

SRClassifier::SRClassifier(){
	FactoryClassifier::getInstance()->registerType("SRClassifier",this);
}

SRClassifier::SRClassifier(string& _type, map<string, string>& params)
{
    type = _type;
	if (params.count("trainFile") > 0)
        load(params["trainFile"]);
}


SRClassifier::SRClassifier(string& _type){
    type = _type;
}

SRClassifier::~SRClassifier(){
	delete omp;
}

void* SRClassifier::createType(string& type){
	if (type == "SRClassifier")
		return new SRClassifier(type);
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;

}

void* SRClassifier::createType(string& type, map<string, string>& params){
    return new SRClassifier(type,params);
}




void SRClassifier::train(cv::Mat trainData, cv::Mat trainLabels){
	arma::fmat trainDataFMat;
	arma::fmat trainLabelsFMat;
	MatrixTools::matToFMat(trainData, trainDataFMat);
	MatrixTools::matToFMat(trainLabels, trainLabelsFMat);
	train(trainDataFMat,trainLabelsFMat);
}

void SRClassifier::test(cv::Mat testData, cv::Mat testLabels){
	arma::fmat testDataFMat;
	arma::fmat testLabelsFMat;
	MatrixTools::matToFMat(testData, testDataFMat);
	MatrixTools::matToFMat(testLabels, testLabelsFMat);
	test(testDataFMat,testLabelsFMat);
}

float SRClassifier::classify( cv::Mat query){
	arma::fmat queryFMat;
	MatrixTools::matToFMat(query, queryFMat);
	return classify(queryFMat);
}


void SRClassifier::train(arma::fmat _trainData, arma::fmat trainLabels){

	//int i = 0;
	trainData = trans(_trainData);
	//l1min::FISTA::option_type opt;
	//opt.eps = 1e-7;
	opt.max_iters = 2000;
	opt.lambda = 0.05;
	//opt.L = 43681; //max(eig_sym(trans(trainData) * trainData));
	//std::cout << "train" << std::endl;
	opt.L = max(eig_sym(trans(trainData) * trainData));
	//std::cout << "ok: opt.L: " <<  opt.L << std::endl;
	omp = new l1min::FISTA(opt);
	// trainData.n_rows = 35.7771+0.34258 opt.L
	/*
	1000 ⁼ 368.408
	5000 ⁼ 1771.89
	8000 ⁼ 2763.15
	25000 ~= 43681
	 */
	labelsCute = arma::fvec(trainLabels);
	numberOfClasses = max(labelsCute)+1;
	 //= m.n_elem;
}



float SRClassifier::classify(arma::fmat query, double* error, arma::fmat* recErrors){

	query = trans(query);
	vector<int> correctGuesses(numberOfClasses,0);
	vector<float> weightGuesses(numberOfClasses,0);

	arma::fvec res = (*omp)(trainData,query);

	arma::uvec indexes = find(abs(res) > 0);

	arma::fvec labelsRight = labelsCute.elem(indexes);
	arma::fvec scoresRight = res.elem(indexes);

	for (unsigned int j = 0; j < labelsRight.n_rows; j++)
		correctGuesses.at(labelsRight.at(j))++;

	float minFromReconstruct = FLT_MAX;
	float detectedLabelFromReconstruct= -1;
	arma::fmat bestReconstruction;
	//double bestMultiFactor = -1;
	if(recErrors != NULL)
		recErrors->set_size(labelsCute.n_rows,2);

	for (int j = 0; j < numberOfClasses; j++){

		if (correctGuesses.at(j) != 0){

			//cout << weightGuesses.at(j)/correctGuesses.at(j) << endl;
			arma::fvec newRes = res;

			for(unsigned int k = 0; k < labelsCute.n_rows; k++){

					float la  = labelsCute.at(k);

					if(recErrors != NULL){
						recErrors->at(k,0) = newRes.at(k);
						recErrors->at(k,1) = k;
					}
					if(la != j)
						newRes.row(k) = 0;
					//else
					//	newRes.row(k) = 1;
			}

			arma::fmat reconstruction = trainData * newRes;



			//double multiFactor = arma::mean(arma::mean(reconstruction/query));
			float reconstructionError;
			reconstructionError = norm(reconstruction-query,2);


			if (reconstructionError<minFromReconstruct){
				minFromReconstruct = reconstructionError;
				detectedLabelFromReconstruct = j;
				bestReconstruction = reconstruction;
				//bestMultiFactor = multiFactor;
			}
		}


	}


	//cout << "detected label: " << (int)detectedLabelFromReconstruct << " error: " <<   minFromReconstruct << endl;
	//cout.precision(10);
	//cout.setf(ios::fixed,ios::floatfield);
	//cout << "[";
	//for(unsigned int k = 0; k < labelsCute.n_rows; k++){
	//	cout << (int)labelsCute.row(k) << "," << abs(res.row(k)) << ";";
	//}

	//cout << "]";
	//getchar();

	if(error != NULL)
		(*error) = minFromReconstruct;


	return detectedLabelFromReconstruct;
}

void SRClassifier::test(arma::fmat testData, arma::fmat testLabels){

}

string SRClassifier::getName(){
	return type;
}

void SRClassifier::changeLabels(arma::fmat trainLabels) {
	labelsCute = arma::fvec(trainLabels);
	numberOfClasses = max(labelsCute)+1;
}

bool SRClassifier::save(string basePath){
	stringstream ss;
	ss << CLASSIFIER_BASE_SAVE_PATH << basePath << TRAINDATA_EXTENSION_SR;

	FileStorage fs(ss.str().c_str(), FileStorage::WRITE);


	cv::Mat trainDataCV;
	MatrixTools::fmatToMat(trainData,trainDataCV);

	cv::Mat labelsCuteCV;
	MatrixTools::fmatToMat(labelsCute,labelsCuteCV);

	fs << "trainDataCV" << trainDataCV;
	fs << "labelsCuteCV" << labelsCuteCV;
	fs << "numberOfClasses" << numberOfClasses;

	int max_iters = opt.max_iters;
	fs << "max_iters" << max_iters;
	fs << "lambda" << opt.lambda;
	fs << "L" << opt.L;
	fs.release();

	return true;
}

bool SRClassifier::load(string basePath){
	stringstream ss;
	ss << CLASSIFIER_BASE_SAVE_PATH << basePath << TRAINDATA_EXTENSION_SR;

	cv::Mat trainDataCV;
	cv::Mat labelsCuteCV;

	FileStorage fs(ss.str().c_str(), FileStorage::READ);

	fs["trainDataCV"] >> trainDataCV;
	fs["labelsCuteCV"] >> labelsCuteCV;
	fs["numberOfClasses"] >> numberOfClasses;

	int max_iters,L;
	float lambda;

	fs["max_iters"] >> max_iters;
	fs["lambda"] >> lambda;
	fs["L"] >> L;

	opt.max_iters = max_iters;
	opt.lambda = lambda;
	opt.L = L;

	arma::fmat labelsCuteFmat;
	MatrixTools::matToFMat(labelsCuteCV,labelsCuteFmat);
	labelsCute = arma::fvec(labelsCuteFmat);

	MatrixTools::matToFMat(trainDataCV,trainData);


	omp = new l1min::FISTA(opt);

	return true;
}
