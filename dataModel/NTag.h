#pragma once
#include "IDataModel.h"

using namespace std;

class NTag : public IDataModel{

public:

	NTag(string SQLTable, map<string,float>* value);
	~NTag();

	bool storeSQL();
	bool loadSQL();

private:

	map<string,float>* value;
	string SQLTable;
	size_t rawSize;

};
