//#include "InterfaceModel.h"
#include "IDataModel.h"
#include "Poco/Data/SQLChannel.h"
#include "Poco/Data/SessionFactory.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/Utility.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

#define SQLFILE "dbs/default.db"

using namespace std;
using namespace Poco::Data;

class DataModelController {

    public:
        DataModelController();
        ~DataModelController();

    private:
        void dropTables();
        void createTables();
};


