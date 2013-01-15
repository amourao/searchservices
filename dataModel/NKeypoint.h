#pragma once
#include "IDataModel.h"

using namespace std;

class NKeypoint : public IDataModel{

public:

	NKeypoint(string SQLTable, vector<pair<vector<float>,vector<float> > >* value);
	~NKeypoint();

	bool storeSQL();
	bool loadSQL(int mediaId);

private:

	vector<pair<vector<float>,vector<float> > >* value;
	string SQLTable;
	size_t rawSize;

	unsigned char* serialize(vector<pair<vector<float>,vector<float> > >* v);
	vector<pair<vector<float>,vector<float> > > unserialize(const unsigned char* buffer);
	void serializeVector(vector<float>* v, unsigned char* buffer, int &offset);
	vector<float> unserializeVector(const unsigned char* buffer, int &offset);

};
