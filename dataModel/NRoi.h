#pragma once
#include "IDataModel.h"

using namespace std;

class NRoi : public IDataModel{

public:

	NRoi(string url, map<string,region> value);
	NRoi();
	~NRoi();

	void* getValue();
	bool storeSQL();
	bool loadSQL(int mediaId);


private:

	map<string,region> value;
	string SQLTable;
	size_t rawSize;

};
