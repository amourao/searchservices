#include "NTag.h"
#include <iostream>

const string SQLFILE = "slb.db";

NTag::NTag(string SQLTable, map<string,float>* value)
{
	this->value = value;
	this->SQLTable = SQLTable;
	this->rawSize = sizeof(int)*value->size()*sizeof(float);

	SQLite::Connector::registerConnector();
	//ifstream f;
	//f.open(SQLFILE.c_str());

	//  if(!f.good())
	// {

	Session ses("SQLite", SQLFILE);
	ses << "DROP TABLE IF EXISTS media", Keywords::now;
	ses << "DROP TABLE IF EXISTS "<< SQLTable, Keywords::now;
	ses << "DROP TABLE IF EXISTS word", Keywords::now;
	ses << "CREATE TABLE media (id INTEGER PRIMARY KEY AUTOINCREMENT, uri TEXT)", Keywords::now;
	ses << "CREATE TABLE word (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, type TEXT)", Keywords::now;
	ses << "CREATE TABLE "<< SQLTable <<" (id INTEGER PRIMARY KEY AUTOINCREMENT, wordId, mediaId, importance REAL, FOREIGN KEY(wordId) REFERENCES word(id), FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses.close();
	// }
}

NTag::~NTag()
{
	SQLite::Connector::unregisterConnector();
}


bool NTag::storeSQL()
{
	Session ses("SQLite", SQLFILE);
	map<string,float>::iterator iter;
	string url= "http://cenas";
	string type = "batata";
	ses << "INSERT INTO media VALUES(null,:uri); SELECT last_insert_rowid() FROM media", Keywords::use(url), Keywords::into(mediaId), Keywords::now;
	for (iter = value->begin(); iter != value->end(); iter++)
	{
		string key = iter->first;
		int wordId;
		ses << "INSERT INTO word VALUES(null,:name,:type); SELECT max(rowid) FROM word", Keywords::use(key), Keywords::use(key), Keywords::into(wordId), Keywords::now;
		cout << key << endl;
		ses << "INSERT INTO "<< SQLTable << " VALUES(null,:wordId, :mediaId, :importance)",Keywords::use(wordId), Keywords::use(mediaId), Keywords::use(iter->second), Keywords::now;
	}
	ses.close();
	return true;
}

bool NTag::loadSQL(int mediaId)
{
	vector<string> keys;
	vector<float> values;
	Session ses("SQLite", SQLFILE);
	ses << "SELECT name, importance FROM "<< SQLTable << ",word WHERE mediaId = :mediaId AND word.Id = teste.wordId", Keywords::use(mediaId), Keywords::into(keys),Keywords::into(values), Keywords::now;
	for(int i = 0; i < keys.size(); i++)
		(*value)[keys[i]] = values[i];
	return true;
}

int main()
{
	map<string,float> v;
	v["bu"] = 5.5;
	v["fef"] = 15.6947;
	v["qwef"] = 62.97;
	IDataModel* idm = new NTag("teste", &v);
	cout << 2;
	idm->storeSQL();
	idm->loadSQL(1);
}
