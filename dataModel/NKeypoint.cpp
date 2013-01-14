#include "NKeypoint.h"
#include <iostream>


const string SQLFILE = "slb.db";

NKeypoint::NKeypoint(string SQLTable, vector<pair<vector<float>,vector<float> > >* value)
{
	this->value = value;
	this->SQLTable = SQLTable;
	this->rawSize = sizeof(int);
	for(int i = 0; i < this->value->size(); i++){
		this->rawSize+= sizeof(int)+(*value)[i].first.size()*sizeof(float);
		this->rawSize+= sizeof(int)+(*value)[i].second.size()*sizeof(float);
	}
	SQLite::Connector::registerConnector();
	ifstream f;
	f.open(SQLFILE.c_str());

	//  if(!f.good())
	// {
	Session ses("SQLite", SQLFILE);
	ses << "DROP TABLE IF EXISTS media", Keywords::now;
	ses << "DROP TABLE IF EXISTS "<< SQLTable, Keywords::now;
	ses << "CREATE TABLE media (id INTEGER PRIMARY KEY AUTOINCREMENT, uri TEXT)", Keywords::now;
	ses << "CREATE TABLE sift (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses.close();
	// }
}

NKeypoint::~NKeypoint()
{
	SQLite::Connector::unregisterConnector();
}


bool NKeypoint::storeSQL()
{

	unsigned char* buffer = serialize(value);
	BLOB data(buffer, rawSize);
	Session ses("SQLite", SQLFILE);
	int mediaId;
	string t= "http://cenas";
	ses << "INSERT INTO media VALUES(null,:uri); SELECT last_insert_rowid() FROM media", Keywords::use(t), Keywords::into(mediaId), Keywords::now;
	ses << "INSERT INTO "<< SQLTable << " VALUES(null,:mediaId, :data)", Keywords::use(mediaId), Keywords::use(data), Keywords::now;
	ses.close();
	return true;
}

bool NKeypoint::loadSQL()
{
	BLOB b;
	Session ses("SQLite", SQLFILE);
	ses << "SELECT data FROM "<< SQLTable << " WHERE mediaId=1", Keywords::into(b), Keywords::now;
	ses.close();
	const unsigned char* buffer = b.rawContent();
	vector<pair<vector<float>,vector<float> > > v = unserialize(buffer);
	return true;
}

unsigned char* NKeypoint::serialize(vector<pair<vector<float>,vector<float> > >* v)
{
	unsigned char* buffer = (unsigned char*) malloc(rawSize);
	int size = v->size();
	memcpy(buffer,&size, sizeof(int));
	cout << size << endl;
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
	vector<pair<vector<float>,vector<float> > > v(size);
	int offset = sizeof(int);
	cout << size << endl;
	for(int i = 0; i < size; i++)
	{
		v[i].first = unserializeVector(buffer, offset);
		v[i].second = unserializeVector(buffer, offset);
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


int main()
{
	string x = "l";
	vector<pair<vector<float>,vector<float> > > v(1);
	vector<float> v1(2);

	v1[0] = 2.7;
	v1[1] = 3.4;
	vector<float> v2(2);
	v2[0] = 5.7;
	v2[1] = 6.4;
	v[0].first = v1;
	v[0].second = v2;

	IDataModel* nv = new NKeypoint(x,&v);
	nv->storeSQL();
	nv->loadSQL();
}
