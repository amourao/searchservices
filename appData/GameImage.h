#include "../dataModel/IDataModel.h"
#include "../dataModel/NVector.h"
<<<<<<< HEAD
#include "../commons/AppConfig.h"
=======
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d


class GameImage : public IDataModel{

public:

	GameImage(string url, int gameId,int roundId,int userId,
			int timeId, int roundAudience, 	int roundExpressionId, int ksvm, float score,
<<<<<<< HEAD
			string username, vector<float> gaborFace, vector<float> histFace, vector<float> reconFace, vector<float> gaborShirt,
			vector<float> histShirt, vector<float> reconShirt);
	GameImage(string url);
=======
			string username, NVector gaborFace, NVector histFace, NVector reconFace, NVector gaborShirt,
			NVector histShirt, NVector reconShirt);
	GameImage();
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d
	~GameImage();

	bool storeSQL();
	bool loadSQL(int mediaId);
	void* getValue();

<<<<<<< HEAD
  int getFlannId();
=======
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d
	int getGameId();
	int getKsvm();
	int getRoundAudience();
	int getRoundExpressionId();
	int getRoundId();
	int getTimeId();
	int getUserId();
	float getScore();
	string getUsername();
<<<<<<< HEAD
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
=======
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d

private:

	int gameId;
	int roundId;
	int userId;
	int timeId;
	int roundAudience;
	int roundExpressionId;
	int ksvm;
<<<<<<< HEAD
	int flannId;
=======
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d
	float score;
	string username;
	NVector gaborFace;
	NVector histFace;
	NVector reconFace;
	NVector gaborShirt;
	NVector histShirt;
	NVector reconShirt;
<<<<<<< HEAD
	
	static bool replace(std::string& str, const std::string& from, const std::string& to);
=======
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d
};
