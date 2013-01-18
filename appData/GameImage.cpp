#include "GameImage.h"

const string SQLFILE = "slb.db";

GameImage::GameImage(unsigned int pk, int gameId,int roundId,int userId,
		int timeId, int roundAudience, 	int roundExpressionId, int ksvm, float score,
		string username)
{
	this->pk = pk;
	this->gameId = gameId;
	this->roundId = roundId;
	this->userId = userId;
	this->timeId = timeId;
	this->roundAudience = roundAudience;
	this->roundExpressionId = roundExpressionId;
	this->ksvm = ksvm;
	this->score = score;
	this->username = username;
}

GameImage::GameImage(){}

GameImage::~GameImage(){

}

bool GameImage::storeSQL(){
	Session ses("SQLite", SQLFILE);
	ses << "INSERT INTO gameImage VALUES(:pk,:gameId,:roundId,:userId,:timeId,:roundAudience,:roundExpressionId,:ksvm,:score,:username)",
			Keywords::use(pk),Keywords::use(gameId),Keywords::use(roundId), Keywords::use(userId), Keywords::use(timeId),Keywords::use(roundAudience),
			Keywords::use(roundExpressionId), Keywords::use(ksvm), Keywords::use(score), Keywords::use(username), Keywords::now;
	ses.close();
	return true;
}

bool GameImage::loadSQL(int pk){
	Session ses("SQLite", SQLFILE);
	this->pk = pk;
	ses << "SELECT * FROM gameImage WHERE pk="<<pk,
			Keywords::into(pk), Keywords::into(gameId),Keywords::into(roundId),Keywords::into(userId),Keywords::into(timeId),Keywords::into(roundAudience),
			Keywords::into(roundExpressionId), Keywords::into(ksvm), Keywords::into(score), Keywords::into(username), Keywords::now;
	ses.close();
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

unsigned int GameImage::getPk() {
	return pk;
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
