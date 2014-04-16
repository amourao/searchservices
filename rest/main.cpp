#include "RestServer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "../commons/LoadConfig.h"
#include "../indexer/IIndexer.h"
#include "../analyser/IAnalyser.h"
#include "IEndpoint.h"

using namespace std;

int str2int (int &i, char const *s, int base = 0)
{
    char *end;
    long  l;
    errno = 0;
    l = strtol(s, &end, base);
    if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
        return -1;
    }
    if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
        return -1;
    }
    if (*s == '\0' || *end != '\0') {
        return -1;
    }
    i = l;
    return 0;
}

int oldMain(int argc, char *argv[])
{
    if(argc != 2){
      cout << "Usage: main <port>" << endl;
      return 1;
    }

    int port;
    int error = str2int(port, argv[1], 10);
    if(error || port < 0){
      cout << "Invalid port" << endl;
      return 1;
    }

    RestServer serv(port);

    return 0;
}

int main(int argc, char *argv[])
{
    if(argc != 2){
      cout << "Usage: main <paramFile>" << endl;
      return 1;
    }

    map<string, string> parameters;
    vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IEndpoint*> endpoints;
	vector<IClassifier*> classifiers;

    string paramFile(argv[1]);
	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

    int port = atoi(parameters["port"].c_str());
    if(port < 0){
      cout << "Invalid port" << endl;
      return 1;
    }

    RestServer serv(port);

    return 0;
}



