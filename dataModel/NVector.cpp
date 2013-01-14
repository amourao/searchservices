#include "NVector.h"
#include <iostream>

const string SQLFILE = "slb.db";

NVector::NVector(string SQLTable, vector<float>* value)
{
	this->value = value;
	this->SQLTable = SQLTable;
	this->rawSize = sizeof(int)+value->size()*sizeof(float);
	SQLite::Connector::registerConnector();
}

NVector::~NVector()
{
	SQLite::Connector::unregisterConnector();
}


bool NVector::storeSQL()
{

	unsigned char* buffer = serializeVector(value);
	BLOB data(buffer, rawSize);
	Session ses("SQLite", SQLFILE);
	int mediaId;
	string t= "http://cenas";
	ses << "INSERT INTO media VALUES(null,:uri); SELECT last_insert_rowid() FROM media", Keywords::use(t), Keywords::into(mediaId), Keywords::now;
	ses << "INSERT INTO "<< SQLTable << " VALUES(null,:mediaId, :data)", Keywords::use(mediaId), Keywords::use(data), Keywords::now;
	ses.close();
	return true;
}

bool NVector::loadSQL()
{
	BLOB b;
	Session ses("SQLite", SQLFILE);
	ses << "SELECT data FROM "<< SQLTable << " WHERE mediaId=1", Keywords::into(b), Keywords::now;
	ses.close();
	const unsigned char* buffer = b.rawContent();
	vector<float> v = unserializeVector(buffer);
	return true;
}

unsigned char* NVector::serializeVector(vector<float>* v)
{
	unsigned char* buffer = (unsigned char*) malloc(rawSize);
	int size = v->size();
	memcpy(buffer,&size, sizeof(int));
	memcpy(buffer+sizeof(int),&((*v)[0]), size*sizeof(float));
	return buffer;
}

vector<float> NVector::unserializeVector(const unsigned char* buffer)
{
	int size;
	memcpy(&size, buffer, sizeof(int));
	vector<float> v(size);
	memcpy(&(v[0]), buffer+sizeof(int), size*sizeof(float));
	return v;
}

int main()
{
	string x = "l";
	vector<float>* v = new vector<float>();
	v->reserve(2);
	v->push_back(2.7);
	v->push_back(3.4);
	IDataModel* nv = new NVector(x,v);
	nv->storeSQL();
	nv->loadSQL();
}
