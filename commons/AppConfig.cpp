#include "AppConfig.h"

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>


AppConfig* AppConfig::instance = NULL;

//Need libjsoncpp
//compile with -ljsoncpp
AppConfig::AppConfig()
{
	Value root;
	Reader reader;
	ifstream ifs("dummy.json");
	string content( (std::istreambuf_iterator<char>(ifs) ),
			(std::istreambuf_iterator<char>()    ) );
	if ( !reader.parse(content,root) )
	{
		std::cout  << "Failed to parse configuration\n" << reader.getFormatedErrorMessages();
		return;
	}
	parseMetadata(root["novasearch"]["metadata"]);
	featuresVec = parseFeatures(root["novasearch"]["featuresVec"]);
	featuresText = parseFeatures(root["novasearch"]["featuresText"]);
	featuresRoi = parseFeatures(root["novasearch"]["featuresRoi"]);
	parseQuerys(root["novasearch"]["queries"]);

}

AppConfig::~AppConfig()
{

}

AppConfig* AppConfig::getInstance()
{
	if (instance == NULL)
		instance = new AppConfig();
	return instance;
};

map<string,vector<pair<string,string> > > AppConfig::getMetadata(){
	return metadata;
}

map<string,vector<map<string,string> > > AppConfig::getFeaturesRoi(){
	return featuresRoi;
}

map<string,vector<map<string,string> > >AppConfig::getFeaturesText(){
	return featuresText;
}

map<string,vector<map<string,string> > > AppConfig::getFeaturesVec(){
	return featuresVec;
}

void AppConfig::parseMetadata(Value appMetadata){
	vector<std::string> appEntitys = appMetadata.getMemberNames();
	for (uint index = 0; index < appMetadata.size(); ++index ){
		vector<pair<string,string> > objectData;
		Value appEntity = appMetadata[appEntitys[index]];
		vector<std::string> appEntrys = appEntity.getMemberNames();
		for(uint i = 0; i < appEntrys.size(); ++i){
			string first = appEntrys[i];
			string second = appEntity[appEntrys[i]].asString();
			pair<string,string> entry(first,second);
			objectData.push_back(entry);
		}
		metadata[appEntitys[index]] = objectData;
	}
}

map<string,vector<map<string,string> > > AppConfig::parseFeatures(Value feature){
	map<string,vector<map<string,string> > > features;
	Value analyser = feature["analyser"];
	for (uint index = 0; index < analyser.size(); ++index)
	{
		Value feature = analyser[index];
		vector<string> members = feature.getMemberNames();
		map<string,string> map;
		for (uint i = 0; i < feature.size(); i++)
		{
			map[members[i]] = feature[members[i]].asString();
		}
		features["analyser"].push_back(map);
	}
	return features;
}

void AppConfig::parseQuerys(Value queries){
	for(uint i = 0; i < queries.size(); i++)
	{
	  this->queries.push_back(queries[i].asString());
	}
}

string AppConfig::getQuery(int index){
	return queries[index];
}

//int main(){
//	map<string,vector<map<string,string> > > test = AppConfig::getInstance()->getFeaturesVec();
//	cout << test["analyser"][1]["name"] << endl;
//}
