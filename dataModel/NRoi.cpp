#include "NRoi.h"
#include <iostream>

const string SQLFILE = "slb.db";

NRoi::NRoi(string SQLTable,  map<string,region>* value)
{


}
NRoi::~NRoi()
{
	SQLite::Connector::unregisterConnector();
}


bool NRoi::storeSQL()
{

	return true;
}

bool NRoi::loadSQL(int mediaId)
{

	return true;
}

int main()
{

}
