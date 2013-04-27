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
	//dropTables();
	//createTables();

}

DataModelController::~DataModelController(){
}

void DataModelController::dropTables(){
	Session ses("SQLite", SQLFILE);
	ses << "BEGIN", Keywords::now;
	ses << "DROP TABLE IF EXISTS media", Keywords::now;
	ses << "DROP TABLE IF EXISTS word", Keywords::now;
	ses << "DROP TABLE IF EXISTS keypoints", Keywords::now;
	ses << "DROP TABLE IF EXISTS vector", Keywords::now;
	ses << "DROP TABLE IF EXISTS FACE_GABOR", Keywords::now;
	ses << "DROP TABLE IF EXISTS FACE_HIST", Keywords::now;
	ses << "DROP TABLE IF EXISTS FACE_RECON", Keywords::now;
	ses << "DROP TABLE IF EXISTS SHIRT_GABOR", Keywords::now;
	ses << "DROP TABLE IF EXISTS SHIRT_HIST", Keywords::now;
	ses << "DROP TABLE IF EXISTS SHIRT_RECON", Keywords::now;
	ses << "DROP TABLE IF EXISTS gameImage", Keywords::now;
	ses << "DROP TABLE IF EXISTS autoAnnotation", Keywords::now;
	ses << "DROP TABLE IF EXISTS annotationType", Keywords::now;
	ses << "DROP TABLE IF EXISTS region", Keywords::now;
	ses << "END", Keywords::now;
	ses.close();
}

void DataModelController::createTables(){

	map<string,IDataModel::type> listOfTypes;
	map<string,IDataModel::type>::iterator iter;
	Session ses("SQLite", SQLFILE);

	ses << "BEGIN", Keywords::now;

	ses << "CREATE TABLE media (id INTEGER PRIMARY KEY AUTOINCREMENT, uri TEXT, partOf, FOREIGN KEY(partOf) REFERENCES media(id))", Keywords::now;
	ses << "CREATE TABLE annotationType (id INTEGER PRIMARY KEY AUTOINCREMENT, description TEXT, typeName TEXT)", Keywords::now;
	ses << "CREATE TABLE word (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, type TEXT, annotationTypeId, FOREIGN KEY(annotationTypeId) REFERENCES annotationType(id))", Keywords::now;
	ses << "CREATE TABLE keypoints (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses << "CREATE TABLE autoAnnotation (wordId INTEGER, mediaId INTEGER, importance REAL, FOREIGN KEY(wordId) REFERENCES word(id), FOREIGN KEY(mediaId) REFERENCES media(id), PRIMARY KEY(wordId,mediaId))", Keywords::now;
	ses << "CREATE TABLE vector (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses << "CREATE TABLE FACE_GABOR (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses << "CREATE TABLE FACE_HIST (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses << "CREATE TABLE FACE_RECON (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses << "CREATE TABLE SHIRT_GABOR (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses << "CREATE TABLE SHIRT_HIST (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses << "CREATE TABLE SHIRT_RECON (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
	ses << "CREATE TABLE region (mediaId PRIMARY KEY, x INTEGER, y INTEGER, height REAL, width REAL, annotationTypeId, FOREIGN KEY(mediaId) REFERENCES media(id), FOREIGN KEY(annotationTypeId) REFERENCES annotationType(id))", Keywords::now;
	ses << "INSERT INTO annotationType VALUES(null,\"face\",\"face\")", Keywords::now;
	ses << "INSERT INTO annotationType VALUES(null,\"carro\",\"carro\")", Keywords::now;
	ses << "CREATE TABLE gameImage (pk INTEGER, gameId INTEGER, roundId INTEGER, userId INTEGER, timeId INTEGER, roundAudience INTEGER, roundExpressionId INTEGER,ksvm INTEGER,score REAL,username TEXT,flannId INTEGER, FOREIGN KEY(pk) REFERENCES media(id))", Keywords::now;
	ses << "END", Keywords::now;

    for (iter = listOfTypes.begin(); iter != listOfTypes.end(); iter++)
    {
            ses << "DROP TABLE IF EXISTS "<< (*iter).first, Keywords::now;
            if((*iter).second == IDataModel::NVECTOR || (*iter).second == IDataModel::NKEYPOINT)
                    ses << "CREATE TABLE "<< (*iter).first << " (id INTEGER PRIMARY KEY AUTOINCREMENT, mediaId, data BLOB, FOREIGN KEY(mediaId) REFERENCES media(id))", Keywords::now;
    }
    ses.close();
}
