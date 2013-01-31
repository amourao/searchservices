#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "Poco/Data/SQLChannel.h"
#include "Poco/Data/SessionFactory.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/Utility.h"
#include "Poco/Data/LOB.h"

using namespace Poco::Data;
using namespace std;

extern const string SQLFile;

typedef struct {
	int x;
	int y;
	float width;
	float height;
	string annotationType;
} region;

class IDataModel {

public:
	
	enum type {
		NVECTOR,
		NKEYPOINT,
		NTAG,
		NROI
	};
	
	IDataModel(string url) {this->url = url; SQLite::Connector::registerConnector();}
	IDataModel() {SQLite::Connector::registerConnector();}
	virtual ~IDataModel() {SQLite::Connector::unregisterConnector();}

	virtual void* getValue() = 0;
	virtual bool storeSQL() = 0;
	virtual bool loadSQL(int mediaId) = 0;



protected:

	string SQLTable;
	string url;

	int getMediaId()
	{
		int mediaId;
		Session ses("SQLite", "slb.db");
		ses << "BEGIN", Keywords::now;
		ses << "SELECT EXISTS(SELECT id FROM media WHERE uri=:url AND partOf IS NULL)",Keywords::use(url), Keywords::into(mediaId), Keywords::now;
		if(mediaId == 0)
			ses << "INSERT INTO media VALUES(null,:url, null)", Keywords::use(url), Keywords::now;
		ses << "SELECT id FROM media WHERE uri=:url AND partOf IS NULL",Keywords::use(url), Keywords::into(mediaId), Keywords::now;
		ses << "END", Keywords::now;
		ses.close();
		return mediaId;
	}

};
