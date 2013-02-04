#include "SRClassifier.h"

SRClassifier::SRClassifier(){
	
}

SRClassifier::~SRClassifier(){
	delete omp;
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

	l1min::FISTA::option_type opt;
	//opt.eps = 1e-7;
	opt.max_iters = 1000;
	opt.lambda = 0.03;
	opt.L = max(eig_sym(trans(trainData) * trainData));
	omp = new l1min::FISTA(opt);
	labelsCute = arma::fvec(trainLabels);
	
	numberOfClasses = max(labelsCute)+1;
	 //= m.n_elem;
}



float SRClassifier::classify(arma::fmat query){
		
	query = trans(query);
	vector<int> correctGuesses(numberOfClasses,0);
	vector<float> weightGuesses(numberOfClasses,0);
	arma::fvec res = (*omp)(trainData,query);
	arma::fvec bestRes;
	arma::uvec indexes = find(abs(res) > 0);

	arma::fvec labelsRight = labelsCute.elem(indexes);
	arma::fvec scoresRight = res.elem(indexes);
	for (unsigned int j = 0; j < labelsRight.n_rows; j++){
		float la  = labelsRight.row(j);
		correctGuesses.at(la)++;
		weightGuesses.at(la) += abs(scoresRight.row(j));	
	}

	float minFromReconstruct = FLT_MAX;
	float detectedLabelFromReconstruct= -1;



	for (int j = 0; j < numberOfClasses; j++){
		if (correctGuesses.at(j) != 0){
			//cout << weightGuesses.at(j)/correctGuesses.at(j) << endl;
			arma::fvec newRes = res;
			for(unsigned int k = 0; k < labelsCute.n_rows; k++){
					float la  = labelsCute.row(k);
					if(la != j)
						newRes.row(k) = 0;
			}
	
			arma::fmat reconstruction = trainData * newRes;
	
			float reconstructionError = norm(reconstruction-query,2);
			if (reconstructionError<minFromReconstruct){
			
				minFromReconstruct = reconstructionError;	
				detectedLabelFromReconstruct = j;
			}
		}
		

	}
	
	cout << "detected label: " << (int)detectedLabelFromReconstruct << " error: " <<   minFromReconstruct << endl;
	cout.precision(10);
	cout.setf(ios::fixed,ios::floatfield);
	cout << "[";
	for(unsigned int k = 0; k < labelsCute.n_rows; k++){
		cout << (int)labelsCute.row(k) << "," << abs(res.row(k)) << ";";
	}
	
	cout << "]";
	getchar();
	return detectedLabelFromReconstruct;
}

void SRClassifier::test(arma::fmat testData, arma::fmat testLabels){
	
}

string SRClassifier::getName(){
	return "SR";
}
