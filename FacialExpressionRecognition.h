#pragma once

#include <string>

using namespace std;

class FacialExpressionRecognition
{
public:

	FacialExpressionRecognition(string trainFile);

	FacialExpressionRecognition(string xmlFile, string svmFile);

	~FacialExpressionRecognition();


	void save(string xmlFile, string svmFile);
	void load(string xmlFile, string svmFile);

private:

	void train(string trainDataFile);
};

