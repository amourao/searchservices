#pragma once
#include "IDataModel.h"

using namespace std;

class NVector : public IDataModel{

public:

	NVector(string SQLTable, vector<float>* value);
	~NVector();

	bool storeSQL();
	bool loadSQL();

private:

	vector<float>* value;
	string SQLTable;
	size_t rawSize;

	unsigned char* serializeVector(vector<float>* v);
	vector<float> unserializeVector(const unsigned char* buffer);

};
