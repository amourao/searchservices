#include "NKeypoint.h"
#include <iostream>

const string SQLFILE = "slb.db";

NKeypoint::NKeypoint(string url, string SQLTable, vector<pair<vector<float>,vector<float> > > value) : IDataModel(url)
{
	this->url = url;
	this->value = value;
	this->SQLTable = SQLTable;
	this->rawSize = sizeof(int);
	for(int i = 0; i < this->value.size(); i++){
		this->rawSize+= sizeof(int)+value[i].first.size()*sizeof(float);
		this->rawSize+= sizeof(int)+value[i].second.size()*sizeof(float);
	}
}

NKeypoint::NKeypoint(string SQLTable) : IDataModel()
{
	this->SQLTable = SQLTable;
}

NKeypoint::~NKeypoint()
{
}

void* NKeypoint::getValue()
{
	return &value;
}

bool NKeypoint::storeSQL()
{
	unsigned char* buffer = serialize(&value);
	BLOB data(buffer, rawSize);
	Session ses("SQLite", SQLFILE);
	int mediaId = getMediaId();
	ses << "INSERT INTO "<< SQLTable << " VALUES(null,:mediaId, :data)", Keywords::use(mediaId), Keywords::use(data), Keywords::now;
	ses.close();
	return true;
}

bool NKeypoint::loadSQL(int mediaId)
{
	BLOB b;
	Session ses("SQLite", SQLFILE);
	ses << "SELECT data FROM "<< SQLTable << " WHERE mediaId="<< mediaId, Keywords::into(b), Keywords::now;
	ses.close();
	const unsigned char* buffer = b.rawContent();
	value = unserialize(buffer);
	return true;
}

unsigned char* NKeypoint::serialize(vector<pair<vector<float>,vector<float> > >* v)
{
	unsigned char* buffer = (unsigned char*) malloc(rawSize);
	int size = v->size();
	memcpy(buffer,&size, sizeof(int));
	int offset = sizeof(int);
	for(int i = 0; i < size; i++)
	{
		serializeVector(&((*v)[i].first), buffer, offset);
		serializeVector(&((*v)[i].second),buffer, offset);
	}
	return buffer;
}

vector<pair<vector<float>,vector<float> > > NKeypoint::unserialize(const unsigned char* buffer)
{
	int size;
	memcpy(&size, buffer, sizeof(int));
	vector<pair<vector<float>,vector<float> > > v;
	int offset = sizeof(int);
	for(int i = 0; i < size; i++)
	{
		vector<float> first = unserializeVector(buffer, offset);
		vector<float> second = unserializeVector(buffer, offset);
		pair<vector<float>,vector<float> > pair(first,second);
		v.push_back(pair);
	}
	return v;
}

void NKeypoint::serializeVector(vector<float>* v, unsigned char* buffer, int &offset)
{
	int size = v->size();
	memcpy(buffer+offset,&size, sizeof(int));
	memcpy(buffer+offset+sizeof(int),&((*v)[0]), size*sizeof(float));
	offset+= sizeof(int)+size*sizeof(float);
}

vector<float> NKeypoint::unserializeVector(const unsigned char* buffer, int &offset)
{
	int size;
	memcpy(&size, buffer+offset, sizeof(int));
	vector<float> v(size);
	memcpy(&(v[0]), buffer+offset+sizeof(int), size*sizeof(float));
	offset+= sizeof(int)+size*sizeof(float);
	return v;
}
