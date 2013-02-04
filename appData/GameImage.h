#include "../dataModel/IDataModel.h"
#include "../dataModel/NVector.h"
#include "../commons/AppConfig.h"


class GameImage : public IDataModel{

public:

	GameImage(string url, int gameId,int roundId,int userId,
			int timeId, int roundAudience, 	int roundExpressionId, int ksvm, float score,
			string username, vector<float> gaborFace, vector<float> histFace, vector<float> reconFace, vector<float> gaborShirt,
			vector<float> histShirt, vector<float> reconShirt);
	GameImage(string url);
	~GameImage();

	bool storeSQL();
	bool loadSQL(int mediaId);
	void* getValue();

  int getFlannId();
	int getGameId();
	int getKsvm();
	int getRoundAudience();
	int getRoundExpressionId();
	int getRoundId();
	int getTimeId();
	int getUserId();
	float getScore();
	string getUsername();
	vector<float> getGaborFace();
	vector<float> getHistFace();
	vector<float> getReconFace();
	vector<float> getGaborShirt();
	vector<float> getHistShirt();
	vector<float> getReconShirt();
	bool serialize(std::ostream &os);
	bool deserialize(std::istream &is);
	void setFlannId(int flannId);
	static vector<GameImage> executeQuery(int id, vector<string> params);

private:

	int gameId;
	int roundId;
	int userId;
	int timeId;
	int roundAudience;
	int roundExpressionId;
	int ksvm;
	int flannId;
	float score;
	string username;
	NVector gaborFace;
	NVector histFace;
	NVector reconFace;
	NVector gaborShirt;
	NVector histShirt;
	NVector reconShirt;
	
	static bool replace(std::string& str, const std::string& from, const std::string& to);
};
