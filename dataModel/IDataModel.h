#pragma once
#include <iostream>
#include <fstream>
#include "Poco/Data/SQLChannel.h"
#include "Poco/Data/SessionFactory.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/Utility.h"
#include "Poco/Data/TypeHandler.h"
#include "Poco/Data/LOB.h"
//#include "Poco/Data/LOBStream.h"

using namespace std;
using namespace Poco::Data;

extern const string SQLFile;

	typedef struct {
		int x;
		int y;
		float width;
		float height;
	} region;

class IDataModel {

public:


	
	enum type {
		NVector,
		NKeypoint,
		NTag,
		NRoi
	};
	
	virtual ~IDataModel() {}
	virtual bool storeSQL() = 0;
	virtual bool loadSQL(int mediaId) = 0;

protected:

	string SQLTable;
	int mediaId;

};
