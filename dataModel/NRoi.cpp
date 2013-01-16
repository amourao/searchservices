#include "NRoi.h"
#include <iostream>

const string SQLFILE = "slb.db";

NRoi::NRoi(string SQLTable,  map<string,region>* value)
{
	this->value = value;
	this->SQLTable = SQLTable;
	SQLite::Connector::registerConnector();
	Session ses("SQLite", SQLFILE);
	ses << "DROP TABLE IF EXISTS media", Keywords::now;
	ses << "DROP TABLE IF EXISTS "<< SQLTable, Keywords::now;
	ses << "DROP TABLE IF EXISTS word", Keywords::now;
	ses << "CREATE TABLE media (id INTEGER PRIMARY KEY AUTOINCREMENT, uri TEXT)", Keywords::now;
	ses << "CREATE TABLE word (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, type TEXT)", Keywords::now;
	ses << "CREATE TABLE "<< SQLTable <<" (id INTEGER PRIMARY KEY AUTOINCREMENT, wordId, mediaId, importance REAL, FOREIGN KEY(wordId) REFERENCES word(id), FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses.close();
}
NRoi::~NRoi()
{
	SQLite::Connector::unregisterConnector();
}


bool NRoi::storeSQL()
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
	return true;
}

bool NRoi::loadSQL(int mediaId)
{

	return true;
}

int main()
{

}
