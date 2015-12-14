#include "NTag.h"
#include <iostream>

const string SQLFILE = "dbs/default.db";

NTag::NTag(string url, map<string,float> value) : IDataModel(url)
{
	this->value = value;
}

NTag::NTag() : IDataModel()
{
}

void* NTag::getValue()
{
	return &value;
}

bool NTag::storeSQL()
{
    Session ses("SQLite", SQLFILE);
    map<string,float>::iterator iter;
    int mediaId = getMediaId();
    ses << "BEGIN",  Keywords::now;
    for (iter = value.begin(); iter != value.end(); iter++)
    {
            string key = iter->first;
            int wordId;
            ses << "INSERT INTO word VALUES(null,:name,:type, null); SELECT max(rowid) FROM word", Keywords::use(key), Keywords::use(key), Keywords::into(wordId), Keywords::now;
            ses << "INSERT INTO autoAnnotation VALUES(:wordId, :mediaId, :importance)",Keywords::use(wordId), Keywords::use(mediaId), Keywords::use(iter->second), Keywords::now;
    }
    ses << "END", Keywords::now;
    ses.close();
    return true;
}

bool NTag::loadSQL(int mediaId)
{
	vector<string> keys;
	vector<float> values;
	Session ses("SQLite", SQLFILE);
	ses << "SELECT name, importance FROM autoAnnotation,word WHERE mediaId = :mediaId AND word.Id=autoAnnotation.wordId", Keywords::use(mediaId), Keywords::into(keys),Keywords::into(values), Keywords::now;
	for(uint i = 0; i < keys.size(); i++)
		value[keys[i]] = values[i];
	ses.close();
	return true;
}

//int main()
//{
//	SQLite::Connector::registerConnector();
//		Session ses("SQLite", "dbs/default.db");
//		ses << "DROP TABLE IF EXISTS media", Keywords::now;
//		ses << "DROP TABLE IF EXISTS word", Keywords::now;
//		ses << "DROP TABLE IF EXISTS keypoints", Keywords::now;
//		ses << "DROP TABLE IF EXISTS vector", Keywords::now;
//		ses << "DROP TABLE IF EXISTS gameImage", Keywords::now;
//		ses << "DROP TABLE IF EXISTS autoAnnotation", Keywords::now;
//		ses << "CREATE TABLE gameImage (pk INTEGER, gameId INTEGER, roundId INTEGER, userId INTEGER, timeId INTEGER, roundAudience INTEGER, roundExpressionId INTEGER,ksvm INTEGER,score REAL,username TEXT)", Keywords::now;
//		ses << "CREATE TABLE media (id INTEGER PRIMARY KEY AUTOINCREMENT, uri TEXT, partOf, FOREIGN KEY(partOf) REFERENCES media(id))", Keywords::now;
//		ses << "CREATE TABLE word (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, type TEXT)", Keywords::now;
//		ses << "CREATE TABLE keypoints (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
//		ses << "CREATE TABLE autoAnnotation (wordId INTEGER, mediaId INTEGER, importance REAL, FOREIGN KEY(wordId) REFERENCES word(id), FOREIGN KEY(mediaId) REFERENCES media(id), PRIMARY KEY(wordId,mediaId))", Keywords::now;
//		ses << "CREATE TABLE vector (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
//		ses.close();
//		SQLite::Connector::unregisterConnector();
//	map<string,float> m;
//	m["A"] = 1.5;
//	m["B"] = 43.1;
//	m["C"] = 654.7;
//	m["D"] = 21.6;
//	m["E"] = 96.4;
//	m["F"] = 12.0;
//	m["G"] = 2.2;
//	m["H"] = 37.14;
//	m["I"] = 41.01;
//	m["J"] = 57.65;
//	m["K"] = 5.3;
//	m["L"] = 73.36;
//	m["M"] = 85.58;
//	m["N"] = 20.01;
//	IDataModel* tags = new NTag("/tag/path", m);
//	tags->storeSQL();
//	IDataModel* tags2 = new NTag();
//	tags2->loadSQL(3);
//	map<string,float> m2 = *static_cast<map<string,float> *> (tags->getValue());
//}

