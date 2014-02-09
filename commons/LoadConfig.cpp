#include "LoadConfig.h"


vector<IIndexer*> LoadConfig::getRegisteredIndeces(string filename){
    Json::Value root;
    Json::Reader reader;

    std::fstream file (filename.c_str(), std::fstream::in | std::fstream::out);

    bool parsingSuccessful = reader.parse( file, root );

    const Json::Value indexes = root["endpoints"]["indexer"];

    return LoadConfig::registerIndeces(indexes);
}

void LoadConfig::load(string filename, map<string,string>& params, vector<IIndexer*>& indeces, vector<IAnalyser*>& analysers){
    Json::Value root;
    Json::Reader reader;

    std::fstream file (filename.c_str(), std::fstream::in | std::fstream::out);

    bool parsingSuccessful = reader.parse( file, root );

    const Json::Value parameters = root["parameters"];

    params = LoadConfig::jsonToDict(parameters);

    const Json::Value indexes = root["endpoints"]["indexer"];

    indeces = LoadConfig::registerIndeces(indexes);

    const Json::Value analysersV = root["endpoints"]["analyser"];

    analysers = LoadConfig::registerAnalysers(analysersV);

}

void LoadConfig::generatePermutations(Json::Value root, vector<map<string,string> >& result){
    vector<vector<string> > lists;

    vector<string> mapIntName;


    vector<map<string,string> > combinations;
    vector<map<string,string> > newCombinations;

    int i = 0;
    for( Json::ValueIterator itr = root.begin(); itr != root.end(); itr++ ){
        vector<string> innerList;
        Json::Value innerArray = root[itr.key().asString()];
        mapIntName.push_back(itr.key().asString());

        if (innerArray.isArray()){
            for ( int j = 0; j < innerArray.size(); j++ ){
                innerList.push_back(innerArray[j].asString());
            }
        } else {
            innerList.push_back(innerArray.asString());
        }
        lists.push_back(innerList);
        i++;
    }



    for(int i = 0; i < lists.at(0).size(); i++){
        map<string,string> inner;
        inner[mapIntName.at(0)] = lists.at(0).at(i);
        combinations.push_back(inner);
    }

    for(int i = 1; i < lists.size(); i++){
        vector<string> next = lists.at(i);
        newCombinations = vector<map<string,string> >();
        for (vector<map<string,string> >::iterator it=combinations.begin(); it!=combinations.end(); ++it){ // *it
            for (vector<string>::iterator it2=next.begin(); it2!=next.end(); ++it2){ // *it2
                (*it)[mapIntName.at(i)] = (*it2);
                newCombinations.push_back((*it));

            }
        }
        combinations = newCombinations;
    }

    /*
    for (int i = 0; i < combinations.size(); i++){
        for (map<string,string>::iterator it=combinations.at(i).begin(); it!=combinations.at(i).end(); ++it){
            cout << it->first << " " << it->second << endl;
        }
    }
    */
    result = combinations;
}

vector<IIndexer*> LoadConfig::registerIndeces(Json::Value plugins){

    vector<IIndexer*> indexers;

    for ( int i = 0; i < plugins.size(); i++ ){
        const Json::Value p = plugins[i];

        string newName = p["newName"].asString();
        string originalName = p["originalName"].asString();

        Json::Value paramsJSON = p["params"];

        map<string,string> params;
        vector<map<string,string> > allParams;

        LoadConfig::generatePermutations(paramsJSON,allParams);

        for( int j = 0; j < allParams.size(); j++){
            map<string,string> params = allParams.at(j);
            stringstream ss;
            ss << newName << "_" << j;
            string newNameId = ss.str();
            IIndexer* originalIndex = (IIndexer*)FactoryIndexer::getInstance()->createType(originalName);
            FactoryIndexer::getInstance()->registerType(newNameId,originalIndex,params);
            IIndexer* readyIndex = (IIndexer*)FactoryIndexer::getInstance()->createType(newNameId);

            indexers.push_back(readyIndex);
        }
    }
    return indexers;
}


vector<IAnalyser*> LoadConfig::registerAnalysers(Json::Value plugins){

    vector<IAnalyser*> indexers;

    for ( int i = 0; i < plugins.size(); i++ ){
        const Json::Value p = plugins[i];

        string newName = p["newName"].asString();
        string originalName = p["originalName"].asString();

        Json::Value paramsJSON = p["params"];

        map<string,string> params;
        vector<map<string,string> > allParams;

        LoadConfig::generatePermutations(paramsJSON,allParams);

        for( int j = 0; j < allParams.size(); j++){
            map<string,string> params = allParams.at(j);
            stringstream ss;
            ss << newName << "_" << j;
            string newNameId = ss.str();
            IAnalyser* originalIndex = (IAnalyser*)FactoryAnalyser::getInstance()->createType(originalName);
            FactoryAnalyser::getInstance()->registerType(newNameId,originalIndex,params);
            IAnalyser* readyIndex = (IAnalyser*)FactoryAnalyser::getInstance()->createType(newNameId);

            indexers.push_back(readyIndex);
        }
    }
    return indexers;
}

map<string,string> LoadConfig::jsonToDict(Json::Value root){
    map<string,string> result;
    for( Json::ValueIterator itr = root.begin(); itr != root.end(); itr++ ){
        if (root[itr.key().asString()].isString()){
            result[itr.key().asString()] = root[itr.key().asString()].asString();
        }
    }
    return result;
}