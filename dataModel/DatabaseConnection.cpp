#include "DatabaseConnection.h"

DatabaseConnection::DatabaseConnection(){
   	Poco::Data::SQLite::Connector::registerConnector();

}

DatabaseConnection::~DatabaseConnection(){
    //dtor
}

std::map<std::string,std::string> DatabaseConnection::getRow(std::string table, std::string fieldsToGet, std::string key, std::string value){
    std::map<std::string,std::string> result;

    Poco::Data::Session ses("SQLite", MSQLFILE );
    Poco::Data::Statement select(ses);
	select << "SELECT " << fieldsToGet << " FROM "<< table << " WHERE " << key << "=\""<< value << "\"";
    select.execute();
    Poco::Data::RecordSet rs(select);
    bool more = rs.moveFirst();
    while (more){
        for (std::size_t col = 0; col < rs.columnCount(); ++col){
            result[rs.columnName(col)] = rs[col].convert<std::string>();
        }
        more = rs.moveNext();
    }
	ses.close();

    return result;
}

std::map<std::string,std::string> DatabaseConnection::getRow(std::string table, std::string key, int value){
    std::map<std::string,std::string> result;

    return result;
}

