#include "DataModelController.h"

const string INDEX = "/index";
const string ANALYSE = "/analyse";
const string STORE = "/store";
const string GET = "GET";
const string PUT = "PUT";
const string POST = "POST";
const string DELETE = "DELETE";

DataModelController::DataModelController(){
	SQLite::Connector::registerConnector();
	/*ifstream f;
	f.open(SQLFILE.c_str());
	if(!f.good())
	 {*/
	dropTables();
	createTables();

}

void DataModelController::newRequest(string method, string endpoint, string url, vector<string> features){
	//TODO
	//FactoryEndpoints

	/*if(!method.compare(GET)){
		if(!endpoint.compare(ANALYSE)){
			//TODO
		}
	}
	else if(!method.compare(PUT)){
		if(!endpoint.compare(STORE)){
			//TODO
		}
		else if(!endpoint.compare(INDEX)){
			//TODO
		}
	}
	else if(!method.compare(POST)){
		//Nothing to be done, yet!
	}
	else if(!method.compare(DELETE)){
		//Nothing to be done, yet!
	}
	else {
		//TODO
		//No such endpoint
	}*/
}

void DataModelController::dropTables(){
	Session ses("SQLite", SQLFILE);
	ses << "DROP TABLE IF EXISTS media", Keywords::now;
	ses << "DROP TABLE IF EXISTS word", Keywords::now;
	ses << "DROP TABLE IF EXISTS auto_annotations", Keywords::now;
	ses.close();
}

void DataModelController::createTables(){
	map<string,IDataModel::type> listOfTypes;
	map<string,IDataModel::type>::iterator iter;
	Session ses("SQLite", SQLFILE);
	ses << "CREATE TABLE media (id INTEGER PRIMARY KEY AUTOINCREMENT, uri TEXT)", Keywords::now;
	ses << "CREATE TABLE word (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, type TEXT)", Keywords::now;
	ses << "CREATE TABLE auto_annotations (id INTEGER PRIMARY KEY AUTOINCREMENT, wordId, mediaId, importance REAL, FOREIGN KEY(wordId) REFERENCES word(id), FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;

	for (iter = listOfTypes.begin(); iter != listOfTypes.end(); listOfTypes++)
	{
		ses << "DROP TABLE IF EXISTS "<< (*iter).first, Keywords::now;
		if((*iter).second == IDataModel::NVector)
			ses << "CREATE TABLE "<< (*iter).first << " (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
		else if((*iter).second == IDataModel::NKeypoint)
			ses << "CREATE TABLE "<< (*iter).first << " (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
		else if((*iter).second == IDataModel::NTag)
			ses << "CREATE TABLE "<< (*iter).first << " (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
		else if((*iter).second == IDataModel::NRoi){
			ses << "CREATE TABLE "<< (*iter).first << " (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
		}
		else{
			//Error
		}
	}

	ses.close();
}
