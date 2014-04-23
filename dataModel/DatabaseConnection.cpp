#include "DatabaseConnection.h"

DatabaseConnection::DatabaseConnection(){


}

DatabaseConnection::~DatabaseConnection(){
    //dtor
}

std::vector<std::map<std::string,std::string> > DatabaseConnection::getRows(std::string table, std::string fieldsToGet, std::vector<std::string> keys, std::vector<std::string> values, bool isOr, int limit){
    Poco::Data::SQLite::Connector::registerConnector();
    std::vector<std::map<std::string,std::string> > results;

    std::stringstream ss;

    for (uint i = 0; i < keys.size(); i++){
        ss << keys.at(i) << "=\""<< values.at(i) << "\" ";
        if (isOr && i < keys.size()-1)
            ss << " OR ";
        else if (!isOr && i < keys.size()-1)
            ss << " AND ";
    }

    if (limit > 0)
        ss << "LIMIT " << limit;

    Poco::Data::Session ses("SQLite", MSQLFILE );
    Poco::Data::Statement select(ses);
	select << "SELECT " << fieldsToGet << " FROM "<< table << " WHERE " << ss.str();
    select.execute();
    Poco::Data::RecordSet rs(select);
    bool more = rs.moveFirst();
    while (more){
        std::map<std::string,std::string> result;
        for (std::size_t col = 0; col < rs.columnCount(); ++col){
            result[rs.columnName(col)] = rs[col].convert<std::string>();
        }
        results.push_back(result);
        more = rs.moveNext();
    }
	ses.close();

    return results;
}


