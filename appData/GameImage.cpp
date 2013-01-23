#include "GameImage.h"

const string SQLFILE = "slb.db";

GameImage::GameImage(string url,int gameId,int roundId,int userId,
		int timeId, int roundAudience, 	int roundExpressionId, int ksvm, float score,
		string username, NVector gaborFace, NVector histFace, NVector reconFace, NVector gaborShirt,
		NVector histShirt, NVector reconShirt) : IDataModel(url)
{
	this->gameId = gameId;
	this->roundId = roundId;
	this->userId = userId;
	this->timeId = timeId;
	this->roundAudience = roundAudience;
	this->roundExpressionId = roundExpressionId;
	this->ksvm = ksvm;
	this->score = score;
	this->username = username;
	this->gaborFace = gaborFace;
	this->histFace = histFace;
	this->reconFace = reconFace;
	this->gaborShirt = gaborShirt;
	this->histShirt = histShirt;
	this->reconShirt = reconShirt;
}

GameImage::GameImage() : IDataModel()
{
	gaborFace = *(new NVector("FACE_GABOR"));
	histFace = *(new NVector("FACE_HIST"));
	reconFace = *(new NVector("FACE_RECON"));
	gaborShirt = *(new NVector("SHIRT_GABOR"));
	histShirt = *(new NVector("SHIRT_HIST"));
	reconShirt = *(new NVector("SHIRT_RECON"));
}

GameImage::~GameImage(){

}

bool GameImage::storeSQL()
{
	int mediaId = getMediaId();
	Session ses("SQLite", SQLFILE);
	ses << "INSERT INTO gameImage VALUES(:mediaId,:gameId,:roundId,:userId,:timeId,:roundAudience,:roundExpressionId,:ksvm,:score,:username)",
			Keywords::use(mediaId),Keywords::use(gameId),Keywords::use(roundId), Keywords::use(userId), Keywords::use(timeId),Keywords::use(roundAudience),
			Keywords::use(roundExpressionId), Keywords::use(ksvm), Keywords::use(score), Keywords::use(username), Keywords::now;
	gaborFace.storeSQL();
	histFace.storeSQL();
	reconFace.storeSQL();
	gaborShirt.storeSQL();
	histShirt.storeSQL();
	reconShirt.storeSQL();
	ses.close();
	return true;
}

bool GameImage::loadSQL(int mediaId)
{
	Session ses("SQLite", SQLFILE);
	ses << "SELECT * FROM gameImage WHERE pk="<<mediaId,
			Keywords::into(mediaId), Keywords::into(gameId),Keywords::into(roundId),Keywords::into(userId),Keywords::into(timeId),Keywords::into(roundAudience),
			Keywords::into(roundExpressionId), Keywords::into(ksvm), Keywords::into(score), Keywords::into(username), Keywords::now;
	ses.close();
	gaborFace.loadSQL(mediaId);
	histFace.loadSQL(mediaId);
	reconFace.loadSQL(mediaId);
	gaborShirt.loadSQL(mediaId);
	histShirt.loadSQL(mediaId);
	reconShirt.loadSQL(mediaId);
	return true;
}

void* GameImage::getValue(){
	return this;
}

int GameImage::getGameId(){
	return gameId;
}

int GameImage::getKsvm() {
	return ksvm;
}

int GameImage::getRoundAudience(){
	return roundAudience;
}

int GameImage::getRoundExpressionId(){
	return roundExpressionId;
}

int GameImage::getRoundId(){
	return roundId;
}

float GameImage::getScore(){
	return score;
}

int GameImage::getTimeId(){
	return timeId;
}

int GameImage::getUserId(){
	return userId;
}

string GameImage::getUsername(){
	return username;
}
