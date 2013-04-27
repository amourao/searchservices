#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <flann/flann.hpp>

#include <Poco/RegularExpression.h>

#include "../../../appData/GameImage.h"
#include "../../FactoryEndpoint.h"
#include "../../IEndpoint.h"

using namespace std;
using namespace Poco;
using namespace flann;

class Codebits : public IEndpoint
{

public:

enum IndexType {
          BY_EXPRESSION,
          BY_FACE,
          BY_SHIRT
};

	Codebits();
	Codebits(string type);
	~Codebits();
	
	void* createType(string &typeId);
	
	void handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp);

private:
  string type;

  static Index<L2<float> > face_gabor;  //FACE_GABOR
  static Index<L2<float> > face_hist;   //FACE_HIST
  static Index<L2<float> > face_recon;  //FACE_RECON
  static Index<L2<float> > shirt_gabor;  //SHIRT_GABOR
  static Index<L2<float> > shirt_hist;  //SHIRT_HIST
  static Index<L2<float> > shirt_recon;  //SHIRT_RECON
  
  static int last_face_gabor_id;
  static int last_face_hist_id;
  static int last_face_recon_id;
  static int last_shirt_gabor_id;
  static int last_shirt_hist_id;
  static int last_shirt_recon_id;
  
  void index(istream& in, map<string, string> parameters);
  void search(map<string, string> parameters, vector<GameImage> &result);
  void latests(map<string, string> parameters, vector<GameImage> &result);
  void best(map<string, string> parameters, vector<GameImage> &result);
  void scoreboard(map<string, string> parameters, vector<GameImage> &result);
  void mybestsmile(map<string, string> parameters, vector<GameImage> &result);
  
  Index<L2<float> >* getIndex(int id);
  vector<float> getVectorForIndex(int id, int media_id);
};

int Codebits::last_face_gabor_id = 0;
int Codebits::last_face_hist_id = 0;
int Codebits::last_face_recon_id = 0;
int Codebits::last_shirt_gabor_id = 0;
int Codebits::last_shirt_hist_id = 0;
int Codebits::last_shirt_recon_id = 0;


Index<L2<float> > Codebits::face_gabor(Matrix<float>(new float[288*1], 1, 288), KDTreeIndexParams(288));
Index<L2<float> > Codebits::face_hist(Matrix<float>(new float[24*1], 1, 24), KDTreeIndexParams(24));
Index<L2<float> > Codebits::face_recon(Matrix<float>(new float[100*1], 1, 100), KDTreeIndexParams(100));
Index<L2<float> > Codebits::shirt_gabor(Matrix<float>(new float[288*1], 1, 288), KDTreeIndexParams(288));
Index<L2<float> > Codebits::shirt_hist(Matrix<float>(new float[24*1], 1, 24), KDTreeIndexParams(24));
Index<L2<float> > Codebits::shirt_recon(Matrix<float>(new float[100*1], 1, 100), KDTreeIndexParams(100));
