#include <AppConfig.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>

AppConfig* AppConfig::instance = NULL;

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
		std::cout  << "Failed to parse configuration\n" << reader.getFormattedErrorMessages();
		return;
	}
	Value appMetadata = root["novasearch"]["metadata"];
	parseMetadata(appMetadata);
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

void AppConfig::parseMetadata(Value appMetadata){
	vector<std::string> appEntitys = appMetadata.getMemberNames();
	for ( int index = 0; index < appMetadata.size(); ++index ){
			vector<pair<string,string> > objectData;
			Value appEntity = appMetadata[appEntitys[index]];
			vector<std::string> appEntrys = appEntity.getMemberNames();
			for(int i = 0; i < appEntrys.size(); ++i){
				string first = appEntrys[i];
				string second = appEntity[appEntrys[i]].asString();
				pair<string,string> entry(first,second);
				objectData.push_back(entry);
			}
			metadata[appEntitys[index]] = objectData;
	}
}

int main(){
	AppConfig::getInstance();
}
