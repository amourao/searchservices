#pragma once
#include "IDataModel.h"

using namespace std;

class NVector : public IDataModel{

public:

	size_t rawSize;

	NVector(string url, string SQLTable, vector<float> value);

	NVector(string url, string SQLTable);

	NVector();
	~NVector();

	void* getValue();
	bool storeSQL();
	bool loadSQL(int mediaId);
	unsigned char* serialize();
	void unserialize(std::istream &is);
	vector<float> getRawVector();


private:

	vector<float> value;
	string SQLTable;


	unsigned char* serializeVector(vector<float>* v);
	vector<float> unserializeVector(const unsigned char* buffer);

};
