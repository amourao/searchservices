#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H

#include <iostream>
#include <string>
#include <vector>
#include <map>


#include "Poco/Data/SQLChannel.h"
#include "Poco/Data/SessionFactory.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/Utility.h"
#include "Poco/Data/RecordSet.h"

#define MSQLFILE "med.db"

struct Person
{
    std::string name;
    std::string address;
    int         age;
};


class DatabaseConnection
{
    public:
        DatabaseConnection();
        virtual ~DatabaseConnection();

        std::map<std::string,std::string> getRow(std::string table, std::string fieldsToGet, std::string key, std::string value);
        std::map<std::string,std::string> getRow(std::string table, std::string key, int value);

    private:

};

#endif // DATABASECONNECTION_H
