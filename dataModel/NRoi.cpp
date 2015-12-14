#include "NRoi.h"
#include <iostream>

const string SQLFILE = "dbs/default.db";

NRoi::NRoi(string url,  map<string,region> value) : IDataModel(url)
{
	this->value = value;
}

NRoi::NRoi() : IDataModel()
{
}


NRoi::~NRoi()
{

}

void* NRoi::getValue(){
	return &value;
}

bool NRoi::storeSQL()
{
	Session ses("SQLite", SQLFILE);
	map<string,region>::iterator iter;
	int mediaId = getMediaId();
	ses << "BEGIN",  Keywords::now;
	for (iter = value.begin(); iter != value.end(); iter++)
	{
		string key = iter->first;
		region region = iter->second;
		int annotationTypeId;
		int wordId;
		int partOfMediaId;
		ses << "INSERT INTO media VALUES(null,:url,"<<mediaId<<")", Keywords::use(url), Keywords::now;
		ses << "SELECT max(rowid) FROM media", Keywords::into(partOfMediaId), Keywords::now;
		ses << "SELECT id FROM annotationType WHERE typeName=\""<<region.annotationType<<"\"", Keywords::into(annotationTypeId), Keywords::now;
		ses << "INSERT INTO word VALUES(null,\"" << key << "\"," << partOfMediaId << ","<< annotationTypeId <<");", Keywords::now;
		ses << "SELECT max(rowid) FROM word", Keywords::into(wordId), Keywords::now;
		ses << "INSERT INTO autoAnnotation VALUES("<< wordId << "," << partOfMediaId << ",null)", Keywords::now;
		ses << "INSERT INTO region VALUES("<<partOfMediaId<<","<<region.x<<","<<region.y<<","<<region.height<<","<<region.width<<","<< annotationTypeId<<")", Keywords::now;
	}
	ses << "END",  Keywords::now;
	ses.close();
	return true;
}

bool NRoi::loadSQL(int mediaId)
{
	Session ses("SQLite", SQLFILE);
	vector<int> partOfs;
	ses << "SELECT id FROM media WHERE partOf="<<mediaId, Keywords::into(partOfs), Keywords::now;
	vector<int>::iterator iter;
	ses << "BEGIN",  Keywords::now;
	for (iter = partOfs.begin(); iter != partOfs.end(); iter++)
	{
		int partOfmediaId = *iter;
		int annotationTypeId;
		vector<int> wordId;
		string key;
		region r;
		ses << "SELECT x,y,height,width,annotationTypeId FROM region WHERE mediaId="<<partOfmediaId,
				Keywords::into(r.x),Keywords::into(r.y),Keywords::into(r.height),Keywords::into(r.width),Keywords::into(annotationTypeId), Keywords::now;
		ses << "SELECT typeName FROM annotationType WHERE id="<<annotationTypeId, Keywords::into(r.annotationType), Keywords::now;
		ses << "SELECT wordId FROM autoAnnotation WHERE mediaId="<<partOfmediaId, Keywords::into(wordId), Keywords::now;
		ses << "SELECT name FROM word WHERE id=:id", Keywords::use(wordId), Keywords::into(key), Keywords::now;
		value[key] = r;
	}
	ses << "END", Keywords::now;
	ses.close();
	return true;
}
