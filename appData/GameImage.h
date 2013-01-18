#include "../dataModel/IDataModel.h"

class GameImage : public IDataModel{

public:

	GameImage(unsigned int pk, int gameId,int roundId,int userId,
			int timeId, int roundAudience, 	int roundExpressionId, int ksvm, float score,
			string username);
	GameImage();
	~GameImage();

	bool storeSQL();
	bool loadSQL(int pk);
	void* getValue();

	unsigned int getPk();
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

	unsigned int pk;
	int gameId;
	int roundId;
	int userId;
	int timeId;
	int roundAudience;
	int roundExpressionId;
	int ksvm;
	float score;
	string username;

};
