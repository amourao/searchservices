#include "dalm.h"
#include "fista.h"
#include "omp.h"
#include "thresholding.h"

#include "SVMClassifier.h"
#include "kNNClassifier.h"

#include <iostream>
#include <fstream>
#include <string>
#include <float.h>
#include <opencv2/highgui/highgui.hpp>

using namespace arma;
using namespace std;


template <typename Matrix>
void normalize_columns(Matrix& matrix) {
    for (uword i = 0; i < matrix.n_cols; ++i) {
        matrix.col(i) /= norm(matrix.col(i), 2);
    }
}

void readFeatures(string f, fmat &featuresM, vector<float> &labels, vector<float> &ids){

	float numSamples = 0;
	int numSamplesTrain = 0;
	float dimensions = 0;
	float id = 0;
 
	std::fstream ifs( f, std::ios::in | std::ios::binary );

	ifs.read( (char*) &dimensions, sizeof(dimensions) );

	ifs.read( (char*) &numSamples, sizeof(numSamples) );

	featuresM = fmat(dimensions, numSamples);

	std::vector<float> features;
	
	for (int i = 0; i < numSamples; i++){
		float label = 0;
		ifs.read( (char*) &id, sizeof(id) );
		ifs.read( (char*) &label, sizeof(label) );
		//cout << id << " " << label << endl;
		labels.push_back((int)label);
		ids.push_back((int)id);
		for (int j = 0; j < dimensions; j++){
			float feature = 0;
			ifs.read( (char*) &feature, sizeof(feature) );
			features.push_back(feature);
		}
		featuresM.col(i) = conv_to<fvec>::from(features);
		features.clear();

	}

	ifs.close();
}


void subtractNeutralFeatures(fmat &features,fmat &neutrals, vector<float> &idsFeatures, vector<float> &idsNeutrals, fmat &result, int subtractSpecialId = -1){
	fvec neutral;
	
	result = features;
	int j = 0;

	if(subtractSpecialId != -1){
		for (int i = 0; i < idsNeutrals.size(); i++){
			if (idsNeutrals.at(i) == 0){
				neutral = neutrals.col(i);
				continue;
			}
		}
	}
	
	for (int i = 0; i < idsFeatures.size(); i++){
		
		if(subtractSpecialId == -1){
			for (int j = 0; j < idsNeutrals.size(); j++){
				//cout << idsNeutrals.at(j) << " " << idsFeatures.at(i) << endl;
				if (idsNeutrals.at(j) == idsFeatures.at(i)){
					neutral = neutrals.col(j);
					
				}
			}
		}
		
		result.col(i) -= neutral;
		result.col(i) = (result.col(i) - min(result.col(i)))/(max(result.col(i))-min(result.col(i)));

		
		//cout << max(result.col(i)) << " " << min(result.col(i)) << endl;
	}
	
	//cout << result << endl;
		
}


void vectorToMat(vector<float> &src, cv::Mat& matrix){	
	matrix.create(1,src.size(),CV_32F);
	for (int i = 0; i < src.size(); i++)
		matrix.at<float>(0,i) = src.at(i);
}

void subtractRowForAll(fmat &fullMatrix, fvec& row){	
	for (int i = 0; fullMatrix.n_rows; i++)
		fullMatrix.row(i) -= row;
}

void fmatToMat(fmat &src, cv::Mat& dst){	
	dst.create(src.n_rows,src.n_cols,CV_32F);
	for (int i = 0; i < src.n_rows; i++)
		for (int j = 0; j < src.n_cols; j++)
			dst.at<float>(i,j) = src(i,j);
}


void matToFMat(cv::Mat &src, fmat& dst){	
	for (int i = 0; i < src.rows; i++)
		for (int j = 0; j < src.cols; j++)
			dst(i,j) = src.at<float>(i,j);
}

void readFeaturesAndDivide(fmat &train,fmat &test, vector<float> &labelsTrain, vector<float> &labelsTest ){

	float numSamples = 0;
	int numSamplesTrain = 0;
	float dimensions = 0;
	string f = "gabor.bin";
 
	std::fstream ifs( f, std::ios::in | std::ios::binary );

	ifs.read( (char*) &dimensions, sizeof(dimensions) );

	//cout << dimensions << endl;

	ifs.read( (char*) &numSamples, sizeof(numSamples) );

	//cout << numSamples << endl;



	vector<float> labels(numSamples);


	numSamplesTrain = numSamples*0.7;
	
	cout << numSamplesTrain << endl;

	//getchar();

	train = fmat(dimensions, numSamplesTrain);

	std::vector<float> features;
	for (int i = 0; i < numSamplesTrain; i++){
		float label = 0;
		ifs.read( (char*) &label, sizeof(label) );
		labelsTrain.push_back((int)label);

		cout << label << " ";
		for (int j = 0; j < dimensions; j++){
			float feature = 0;
			ifs.read( (char*) &feature, sizeof(feature) );
			features.push_back(feature);
		}
		train.col(i) = conv_to<fvec>::from(features);
		features.clear();

	}

	test = fmat(dimensions, numSamples - numSamplesTrain);

	for (int i = numSamplesTrain; i < numSamples; i++){
		float label = 0;
		ifs.read( (char*) &label, sizeof(label) );
		labelsTest.push_back((int)label);
		for (int j = 0; j < dimensions; j++){
			float feature = 0;
			ifs.read( (char*) &feature, sizeof(feature) );
			features.push_back(feature);
		}
		test.col(i-numSamplesTrain) = conv_to<fvec>::from(features);
		features.clear();
	}

	ifs.close();
}


float classifyPedro(fmat& train, fvec& b, fvec& labelsCute, l1min::FISTA& omp){
	
	vector<int> correctGuesses(8,0);
	vector<float> weightGuesses(8,0);
	fvec res = omp(train, b);

	uvec indexes = find(abs(res) > 0);

	//cout << labelsCute.elem(indexes) << endl;
	//cout << res.elem(indexes) << endl;

	fvec labelsRight = labelsCute.elem(indexes);
	fvec scoresRight = res.elem(indexes);
	for (int j = 0; j < labelsRight.n_rows; j++){
	
		float la  = labelsRight.row(j);
		correctGuesses.at(la)++;
		weightGuesses.at(la) += abs(scoresRight.row(j));	
	}

	//weightGuesses /= correctGuesses;
	float max = -1;
	int detectedLabel = -1;

	float maxFromScore = -9999;
	int detectedLabelFromScore = -1;

	float minFromReconstruct = FLT_MAX;
	float detectedLabelFromReconstruct= -1;

	for (int j = 0; j < correctGuesses.size(); j++){
	
		if (correctGuesses.at(j) != 0){
			//cout << weightGuesses.at(j)/correctGuesses.at(j) << endl;
			
			fvec newRes = res;
			for(int k = 0; k < labelsCute.n_rows; k++){
					float la  = labelsCute.row(k);
					if(la != j)
						newRes.row(k) = 0;
			}
	
			fmat reconstruction = train * newRes;
	
			float reconstructionError = norm(reconstruction-b,2);
			//cout << "j: " << j << " error: " << reconstructionError << endl;
			//getchar();
			if (reconstructionError<minFromReconstruct){
			
				minFromReconstruct = reconstructionError;	
				detectedLabelFromReconstruct = j;
			}
		}
	}
	return detectedLabelFromReconstruct;
	
}

void trainAndTest(fmat train, fmat test ,vector<float> labelsTrain, vector<float> labelsTest){
	
	int we = 0;

	//cout << "a "  << we++ << endl;

	vector<float> tpsFromRec(8,0);
	vector<float> fpsFromRec(8,0);
	vector<float> tpsFromSVM(8,0);
	vector<float> fpsFromSVM(8,0);
	vector<float> tpsFromkNN(8,0);
	vector<float> fpsFromkNN(8,0);
	vector<float> tpsFromkNN3(8,0);
	vector<float> fpsFromkNN3(8,0);
	
	SVMClassifier svm;
	kNNClassifier knn;
		
	cv::Mat trainDataMat,testDataMat;	
	cv::Mat trainLabelsMat,testLabelsMat;	
	
	//cout << "a "  << we++ << endl;
	fmatToMat(train,trainDataMat);
	fmatToMat(test,testDataMat);
	//cout << "a "  << we++ << endl;
	vectorToMat(labelsTrain,trainLabelsMat);
	vectorToMat(labelsTest,testLabelsMat);

//cout << "a "  << we++ << endl;
	transpose(trainDataMat,trainDataMat);
	transpose(testDataMat,testDataMat);
	//cout << "a "  << we++ << endl;
	svm.train(trainDataMat,trainLabelsMat);
	knn.train(trainDataMat,trainLabelsMat);
//cout << "a "  << we++ << endl;
	l1min::FISTA::option_type opt;
	//opt.eps = 1e-7;
	opt.max_iters = 1000;
	opt.lambda = 0.03;
	opt.L = max(eig_sym(trans(train) * train));

	l1min::FISTA omp(opt);

	fvec labelsCute = conv_to<fvec>::from(labelsTrain);

	//normalize_columns(train);
	//normalize_columns(test);

	for (int i = 0; i < test.n_cols; i++){
		//int er = 0;
		//cout << er++  << " "  << we << " " << i << endl;
		
		float detectedLabelFromSVM =svm.classify(testDataMat.row(i));
		//cout<< er++  << " " << we << " " << i << endl;
		float detectedLabelFromkNN =knn.classify(testDataMat.row(i));

		float detectedLabelFromkNN3 =knn.classify(testDataMat.row(i),3);
		//cout << er++  << " "  << we << " " << i << endl;
		//cout << i << " " << test.n_cols << " " << " ok" << endl;

		fvec b  = test.col(i);
		//cout << er++  << " "  << we << " " << i << endl;
		float detectedLabelFromReconstruct = classifyPedro(train, b, labelsCute, omp);
		//cout << er++  << " "  << we << " " << i << endl;
		//cout << detectedLabel << endl;
		//cout << labelsTest.at(i)  << ": " << max << " " << detectedLabel << " " << maxFromScore << " " << detectedLabelFromScore << endl;
		if (detectedLabelFromReconstruct == labelsTest.at(i))
			tpsFromRec.at(labelsTest.at(i))++;
		else
			fpsFromRec.at(labelsTest.at(i))++;

		if (detectedLabelFromSVM == labelsTest.at(i))
			tpsFromSVM.at(labelsTest.at(i))++;
		else
			fpsFromSVM.at(labelsTest.at(i))++;

		if (detectedLabelFromkNN == labelsTest.at(i))
			tpsFromkNN.at(labelsTest.at(i))++;
		else
			fpsFromkNN.at(labelsTest.at(i))++;

		if (detectedLabelFromkNN3 == labelsTest.at(i))
			tpsFromkNN3.at(labelsTest.at(i))++;
		else
			fpsFromkNN3.at(labelsTest.at(i))++;
	}
	
	
	for (int j = 0; j < tpsFromRec.size(); j++){
		cout << tpsFromRec.at(j) << "\t";
	}
	cout << endl;

	for (int j = 0; j < fpsFromRec.size(); j++){
		cout << fpsFromRec.at(j) << "\t";
	}
	cout << endl<< endl;

	for (int j = 0; j < tpsFromRec.size(); j++){
		cout << tpsFromSVM.at(j) << "\t";
	}
	cout << endl;

	for (int j = 0; j < fpsFromRec.size(); j++){
		cout << fpsFromSVM.at(j) << "\t";
	}
	cout << endl<< endl;

		for (int j = 0; j < tpsFromRec.size(); j++){
		cout << tpsFromkNN.at(j) << "\t";
	}
	cout << endl;

	for (int j = 0; j < fpsFromRec.size(); j++){
		cout << fpsFromkNN.at(j) << "\t";
	}
		cout << endl<< endl;

		for (int j = 0; j < tpsFromRec.size(); j++){
		cout << tpsFromkNN3.at(j) << "\t";
	}
	cout << endl;

	for (int j = 0; j < fpsFromRec.size(); j++){
		cout << fpsFromkNN3.at(j) << "\t";
	}
	cout << endl;

}

int mainClassifiers(int argc, char** argv){


	if (argc < 3)
		cout << "Not enough args" << endl;

	fmat train;
	fmat test;
	vector<float> labelsTrain;
	vector<float> labelsTest;
	vector<float> idsTrain;
	vector<float> idsTest;
	string trainPath= argv[1];
	string testPath = argv[2];
	
	fmat neutralTrain;
	vector<float> labelsNeutralTrain;
	vector<float> idsNeutralTrain;

	fmat neutral;
	vector<float> labelsNeutral;
	vector<float> idsNeutral;
	
	vector<string> neutrals;
	
	for (int i = 3; i < argc; i++){
		neutrals.push_back(string(argv[i]));
	}
	//int j = 0;
	//cout << j++ << endl;
	readFeatures(trainPath,train,labelsTrain,idsTrain);
//cout << j++ << endl;
	readFeatures(testPath,test,labelsTest,idsTest);
//cout << j++ << endl;
	trainAndTest(train,test,labelsTrain, labelsTest);
//	cout << j++ << endl;
	for (int i = 0; i < neutrals.size(); i+=2){
		//cout << j++ << endl;
		fmat train_n (train);
		fmat test_n  (test);
		
		idsNeutral.clear();
		labelsNeutral.clear();
		neutral.clear();
		

		//cout << neutrals.at(i)<< "  " << neutrals.at(i+1) << endl;
		readFeatures(neutrals.at(i) ,neutralTrain,labelsNeutralTrain,idsNeutralTrain);
		readFeatures(neutrals.at(i+1) ,neutral,labelsNeutral,idsNeutral);
		

		//cout << idsNeutral.size() << endl; 
		subtractNeutralFeatures(train_n,neutralTrain, idsTrain, idsNeutralTrain, train_n);
		subtractNeutralFeatures(test_n,neutral, idsTest, idsNeutral, test_n);
		
		trainAndTest(train_n, test_n ,labelsTrain, labelsTest);
		
	}
//	cout << j++ << endl;
	//readFeatures(trainPath,train,labelsTrain,idsTrain);
	
	
	
	//  getchar();
	return 0;
}
