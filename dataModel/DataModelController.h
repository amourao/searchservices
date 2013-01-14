//#include "InterfaceModel.h"
#include "Poco/Data/SQLChannel.h"
#include "Poco/Data/SessionFactory.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/Utility.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace Poco::Data;

class DataModelController {

    public:
        DataModelController();
        ~DataModelController();

        void newRequest(string method, string endpoint, string url, vector<string> features);

    private:
        void dropTables();
        void createTables();
};

        
