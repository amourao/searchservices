#include "NVector.h"
#include <iostream>

const string SQLFILE = "slb.db";

NVector::NVector(string url, string SQLTable, vector<float> value) : IDataModel(url)
{
	this->value = value;
	this->SQLTable = SQLTable;
	this->rawSize = sizeof(int)+value.size()*sizeof(float);
}

NVector::NVector(string SQLTable) : IDataModel()
{
	this->SQLTable = SQLTable;
}

NVector::NVector() : IDataModel()
{
}


NVector::~NVector()
{
}

void* NVector::getValue(){
	return &value;
}

bool NVector::storeSQL()
{

	unsigned char* buffer = serializeVector(&value);
	BLOB data(buffer, rawSize);
	Session ses("SQLite", SQLFILE);
	int mediaId = getMediaId();
	ses << "INSERT INTO "<< SQLTable << " VALUES(null,:mediaId, :data)", Keywords::use(mediaId), Keywords::use(data), Keywords::now;
	ses.close();
	return true;
}

bool NVector::loadSQL(int mediaId)
{
	BLOB blob;
	Session ses("SQLite", SQLFILE);
	ses << "SELECT data FROM "<< SQLTable << " WHERE mediaId="<< mediaId, Keywords::into(blob), Keywords::now;
	ses.close();
	const unsigned char* buffer = blob.rawContent();
	value = unserializeVector(buffer);
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

unsigned char* NVector::serialize()
{
	return serializeVector(&value);
}

void NVector::unserialize(std::istream &is)
{
	int vlen;
    is.read((char*)&vlen, sizeof(int));
    value.resize(vlen);
    is.read((char*)&value[0],vlen);
}
