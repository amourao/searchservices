#include "GameImage.h"

const string SQLFILE = "dbs/default.db";

GameImage::GameImage(string url,int gameId,int roundId,int userId,
		int timeId, int roundAudience, 	int roundExpressionId, int ksvm, float score,
		string username, vector<float> gaborFace, vector<float> histFace, vector<float> reconFace, vector<float> gaborShirt,
		vector<float> histShirt, vector<float> reconShirt) : IDataModel(url)

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
	this->gaborFace = *(new NVector(url,"FACE_GABOR",gaborFace));
	this->histFace = *(new NVector(url,"FACE_HIST",histFace));
	this->reconFace = *(new NVector(url,"FACE_RECON",reconFace));
	this->gaborShirt = *(new NVector(url,"SHIRT_GABOR",gaborShirt));
	this->histShirt = *(new NVector(url,"SHIRT_HIST",histShirt));
	this->reconShirt = *(new NVector(url,"SHIRT_RECON",reconShirt));
}

GameImage::GameImage(string url) : IDataModel(url)
{
	gaborFace = *(new NVector(url, "FACE_GABOR"));
	histFace = *(new NVector(url, "FACE_HIST"));
	reconFace = *(new NVector(url, "FACE_RECON"));
	gaborShirt = *(new NVector(url, "SHIRT_GABOR"));
	histShirt = *(new NVector(url, "SHIRT_HIST"));
	reconShirt = *(new NVector(url, "SHIRT_RECON"));

}

GameImage::~GameImage(){

}

void GameImage::setFlannId(int flannId)
{
	this->flannId = flannId;
}

bool GameImage::storeSQL()
{
	int mediaId = getMediaId();
	cout << "MediaID" << mediaId << endl;
	Session ses("SQLite", SQLFILE);

	ses << "BEGIN" << Keywords::now;
	ses << "INSERT INTO gameImage VALUES(:mediaId,:gameId,:roundId,:userId,:timeId,:roundAudience,:roundExpressionId,:ksvm,:score,:username,:flannId)",
			Keywords::use(mediaId),Keywords::use(gameId),Keywords::use(roundId), Keywords::use(userId), Keywords::use(timeId),Keywords::use(roundAudience),
			Keywords::use(roundExpressionId), Keywords::use(ksvm), Keywords::use(score), Keywords::use(username),Keywords::use(flannId), Keywords::now;

	gaborFace.storeSQL();
	histFace.storeSQL();
	reconFace.storeSQL();
	gaborShirt.storeSQL();
	histShirt.storeSQL();
	reconShirt.storeSQL();
	ses << "END" << Keywords::now;

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

bool GameImage::serialize(std::ostream &os)
{
	os.write((char*)&gameId,sizeof(int));
	os.write((char*)&roundId,sizeof(int));
	os.write((char*)&userId,sizeof(int));

	os.write((char*)&timeId,sizeof(int));

	os.write((char*)&roundAudience,sizeof(int));
	os.write((char*)&roundExpressionId,sizeof(int));
	os.write((char*)&score,sizeof(float));
	os.write((char*)&ksvm,sizeof(int));

	int vlen = username.size();
	os.write((char*)&vlen,sizeof(int));
	os.write(username.c_str(),vlen);

	char* serializeGaborFace = reinterpret_cast<char*>(gaborFace.serialize());
	char* serializeHistFace = reinterpret_cast<char*>(histFace.serialize());
	char* serializeReconFace = reinterpret_cast<char*>(reconFace.serialize());
	char* serializeGaborShirt = reinterpret_cast<char*>(gaborShirt.serialize());
	char* serializeHistShirt = reinterpret_cast<char*>(histShirt.serialize());
	char* serializeReconShirt = reinterpret_cast<char*>(reconShirt.serialize());

	vlen = sizeof(serializeGaborFace) / sizeof(serializeGaborFace[0]);
	os.write((char*)&vlen,sizeof(int));
	os.write((char*)&(serializeGaborFace[0]),vlen);

	vlen = sizeof(serializeHistFace) / sizeof(serializeHistFace[0]);
	os.write((char*)&vlen,sizeof(int));
	os.write((char*)&(serializeHistFace[0]),vlen);

	vlen = sizeof(serializeReconFace) / sizeof(serializeReconFace[0]);
	os.write((char*)&vlen,sizeof(int));
	os.write((char*)&(serializeReconFace[0]),vlen);

	vlen = sizeof(serializeGaborShirt) / sizeof(serializeGaborShirt[0]);
	os.write((char*)&vlen,sizeof(int));
	os.write((char*)&(serializeGaborShirt[0]),vlen);

	vlen = sizeof(serializeHistShirt) / sizeof(serializeHistShirt[0]);
	os.write((char*)&vlen,sizeof(int));
	os.write((char*)&(serializeHistShirt[0]),vlen);

	vlen = sizeof(serializeReconShirt) / sizeof(serializeReconShirt[0]);
	os.write((char*)&vlen,sizeof(int));
	os.write((char*)&(serializeReconShirt[0]),vlen);

	return true;
}

bool GameImage::deserialize(std::istream &is)
{

	is.read((char*)&gameId,sizeof(int));
	cout << gameId << endl;
	is.read((char*)&roundId,sizeof(int));
	cout << roundId << endl;
	is.read((char*)&userId,sizeof(int));

	is.read((char*)&timeId,sizeof(int));

	is.read((char*)&roundAudience,sizeof(int));
	is.read((char*)&roundExpressionId,sizeof(int));
	is.read((char*)&score,sizeof(float));
	is.read((char*)&ksvm,sizeof(int));

	int vlen;
	is.read((char*)&vlen,sizeof(int));
	cout << vlen << endl;
	username.resize(vlen);
	is.read(&username[0],vlen);

	gaborFace.unserialize(is);
    cout << gaborFace.getRawVector().size() << endl;
    uint i =0;
    for(i = 0; i < gaborFace.getRawVector().size(); i++){
    	cout << gaborFace.getRawVector()[i] << " ";
    }
	histFace.unserialize(is);
	reconFace.unserialize(is);
	gaborShirt.unserialize(is);
	histShirt.unserialize(is);
	reconShirt.unserialize(is);

	return true;
}

void* GameImage::getValue(){
	return this;
}

int GameImage::getFlannId(){
  return flannId;
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

vector<float> GameImage::getGaborFace(){
  return gaborFace.getRawVector();
}

vector<float> GameImage::getHistFace(){
  return histFace.getRawVector();
}

vector<float> GameImage::getReconFace(){
  return reconFace.getRawVector();
}

vector<float> GameImage::getGaborShirt(){
  return gaborShirt.getRawVector();
}

vector<float> GameImage::getHistShirt(){
  return histShirt.getRawVector();
}

vector<float> GameImage::getReconShirt(){
  return reconShirt.getRawVector();
}

void GameImage::executeQuery(int id, vector<string> params, vector<GameImage> &result)
{
	vector<int> mediaIds;
	vector<int> gameIds;
	vector<int> roundIds;
	vector<int> userIds;
	vector<int> timeIds;
	vector<int> roundAudience;
	vector<int> roundExpressionId;
	vector<int> ksvms;
	vector<int> flannIds;
	vector<float> scores;
	NVector gaborFace;
	NVector histFace;
	NVector reconFace;
	NVector gaborShirt;
	NVector histShirt;
	NVector reconShirt;
	vector<string> usernames;
	Session ses("SQLite", SQLFILE);
	//ses << AppConfig::getInstance()->getQuery(id);
	vector<string>::iterator it;
	string q = AppConfig::getInstance()->getQuery(id);
	for(it = params.begin(); it != params.end(); it++)
	{
		uint pos = q.find(":",0);
		uint pos2 = q.find(" ", pos);
		if(pos2 == q.npos)
			pos2 = q.size();
		string param = q.substr(pos, pos2-pos);
		replace(q, param, *it);
	}
	//for(it = params.begin(); it != params.end(); it++)
	//{
	//	ses << Keywords::use(*it);
	// }
	//int x = 5;
	cout << q << endl;
	ses << q, Keywords::into(mediaIds), Keywords::into(gameIds), Keywords::into(roundIds),
			Keywords::into(userIds), Keywords::into(timeIds), Keywords::into(roundAudience),
			Keywords::into(roundExpressionId), Keywords::into(ksvms), Keywords::into(scores),
			Keywords::into(usernames), Keywords::into(flannIds), Keywords::now;
	cout << "Query done" << endl;
	result.reserve(mediaIds.size());
	for(uint i = 0; i < mediaIds.size(); i++)
	{
		string url;
		ses << "SELECT uri FROM media WHERE id=" << mediaIds[i], Keywords::into(url),Keywords::now;

	gaborFace = *(new NVector(url, "FACE_GABOR"));
	histFace = *(new NVector(url, "FACE_HIST"));
	reconFace = *(new NVector(url, "FACE_RECON"));
	gaborShirt = *(new NVector(url, "SHIRT_GABOR"));
	histShirt = *(new NVector(url, "SHIRT_HIST"));
	reconShirt = *(new NVector(url, "SHIRT_RECON"));

		gaborFace.loadSQL(mediaIds[i]);
 		histFace.loadSQL(mediaIds[i]);
		reconFace.loadSQL(mediaIds[i]);
		gaborShirt.loadSQL(mediaIds[i]);
		histShirt.loadSQL(mediaIds[i]);
		reconShirt.loadSQL(mediaIds[i]);
		GameImage gm(url,gameIds[i], roundIds[i], userIds[i], timeIds[i], roundAudience[i],
				roundExpressionId[i], ksvms[i], scores[i], usernames[i], gaborFace.getRawVector(), histFace.getRawVector(),
				reconFace.getRawVector(),gaborShirt.getRawVector(), histShirt.getRawVector(), reconShirt.getRawVector());
		gm.setFlannId(flannIds[i]);
		result.push_back(gm);
	}
	ses.close();
}

bool GameImage::replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

int GameImage::size(){
	return (8 * sizeof(int)) + sizeof(float) + username.size() + gaborFace.rawSize + histFace.rawSize
			+ reconFace.rawSize + gaborShirt.rawSize + histFace.rawSize + reconShirt.rawSize;
}

