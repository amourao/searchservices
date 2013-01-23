#include "../dataModel/IDataModel.h"
#include "../dataModel/NVector.h"


class GameImage : public IDataModel{

public:

	GameImage(string url, int gameId,int roundId,int userId,
			int timeId, int roundAudience, 	int roundExpressionId, int ksvm, float score,
			string username, NVector gaborFace, NVector histFace, NVector reconFace, NVector gaborShirt,
			NVector histShirt, NVector reconShirt);
	GameImage();
	~GameImage();

	bool storeSQL();
	bool loadSQL(int mediaId);
	void* getValue();

	int getGameId();
	int getKsvm();
	int getRoundAudience();
	int getRoundExpressionId();
	int getRoundId();
	int getTimeId();
	int getUserId();
	float getScore();
	string getUsername();

private:

	int gameId;
	int roundId;
	int userId;
	int timeId;
	int roundAudience;
	int roundExpressionId;
	int ksvm;
	float score;
	string username;
	NVector gaborFace;
	NVector histFace;
	NVector reconFace;
	NVector gaborShirt;
	NVector histShirt;
	NVector reconShirt;
};
