#include "NTag.h"
#include <iostream>

const string SQLFILE = "slb.db";

NTag::NTag(string SQLTable, map<string,float>* value)
{
	this->value = value;
	this->SQLTable = SQLTable;
	this->rawSize = sizeof(int)*value->size()*sizeof(float);

	SQLite::Connector::registerConnector();
	ifstream f;
	f.open(SQLFILE.c_str());

	//  if(!f.good())
		// {
	int count = 0;
	Session ses("SQLite", SQLFILE);
	ses << "DROP TABLE IF EXISTS media", Keywords::now;
	ses << "DROP TABLE IF EXISTS "<< SQLTable, Keywords::now;
	ses << "CREATE TABLE media (id INTEGER PRIMARY KEY AUTOINCREMENT, uri TEXT)", Keywords::now;
	ses << "CREATE TABLE "<< SQLTable << " (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses.close();
	// }
}

NTag::~NTag()
{
	SQLite::Connector::unregisterConnector();
}


bool NTag::storeSQL()
{
	return true;
}

bool NTag::loadSQL()
{
	return true;
}


int main()
{

}
