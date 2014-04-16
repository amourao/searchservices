#ifndef LOADCONFIG_H
#define LOADCONFIG_H


#include "../indexer/FactoryIndexer.h"
#include "../indexer/IIndexer.h"

#include "../analyser/FactoryAnalyser.h"
#include "../analyser/IAnalyser.h"

#include "../rest/FactoryEndpoint.h"
#include "../rest/IEndpoint.h"

#include <iomanip>
#include <jsoncpp/json/json.h>


class LoadConfig
{
    public:
        static void load(string filename, map<string,string>& params, vector<IIndexer*>& indeces, vector<IAnalyser*>& analysers);
        static vector<IIndexer*> getRegisteredIndeces(string filename);

    private:
        static vector<IIndexer*> registerIndeces(Json::Value root);
        static vector<IAnalyser*> registerAnalysers(Json::Value plugins);
        static vector<IEndpoint*> registerEndpoints(Json::Value plugins);
        static void generatePermutations(Json::Value root, vector<map<string,string> >& result);
        static map<string,string> jsonToDict(Json::Value root);

};

#endif // LOADCONFIG_H
