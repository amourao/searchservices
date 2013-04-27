#include "Codebits.h"

static Codebits codebitsFactory;

Codebits::Codebits(string type){
  this->type = type;
}

Codebits::Codebits(){
	FactoryEndpoint::getInstance()->registerType("/search",this);
	FactoryEndpoint::getInstance()->registerType("/latests",this);
	FactoryEndpoint::getInstance()->registerType("/best",this);
	FactoryEndpoint::getInstance()->registerType("/scoreboard",this);
	FactoryEndpoint::getInstance()->registerType("/mybestsmile",this);
	FactoryEndpoint::getInstance()->registerType("9090",this);
	
  face_gabor.buildIndex();
  face_hist.buildIndex();
  face_recon.buildIndex();
  shirt_gabor.buildIndex();
  shirt_hist.buildIndex();
  shirt_recon.buildIndex();
	
  face_gabor.removePoint(0);
  face_hist.removePoint(0);
  face_recon.removePoint(0);
  shirt_gabor.removePoint(0);
  shirt_hist.removePoint(0);
  shirt_recon.removePoint(0);
}

Codebits::~Codebits()
{}

int to_int(const string &numstr, bool &error) {
  if (error) return 0;
  stringstream ss(numstr);
  int res; ss >> res;
  error = error || ss.fail();
  return res;
}

int check_int_val(const string& qstr, int min, int max, bool& error) {
  if (error) return 0;
  
  RegularExpression re("([0-9]+)");
  RegularExpression::MatchVec match;
	
  error = !re.match(qstr, 0, match);
  cout << "found " << error << " " << endl;
  if (error) {
    return 0;
  } else {
    cout << "found " << qstr.substr(match[1].offset, match[1].length) << endl;
    int res = to_int(qstr.substr(match[1].offset, match[1].length), error);
    error = error || res < min || res > max;
    return res;
  }
}

string check_string_val(const string& qstr, bool& error) {
	if (error) return "";
	
	RegularExpression re("([a-zA-Z ]+)");
	RegularExpression::MatchVec match;
	
	error = !re.match(qstr, 0, match);
	if (error) {
		return "";
	} else {
		cout << "found " << qstr.substr(match[1].offset, match[1].length) << endl;
		return qstr.substr(match[1].offset, match[1].length);
	}
}

// build json response
void dump_json(/*const*/ vector<GameImage>& result, ostream& out) {
    out << "{ \"rank\": [" << endl;
    for (size_t i = 0; i != result.size(); i++) {
        out << "{\"GameId\": \"" << result[i].getGameId() << "\"," << endl;
        out << "\"id\": \"" << result[i].getPublicMediaId() << "\"," << endl;
        out << "\"pk\": \"" << result[i].getUrl() << "\"," << endl;
        out << "\"RoundId\": \"" << result[i].getRoundId() << "\"," << endl;
        out << "\"UserId\": \"" << result[i].getUserId() << "\"," << endl;
        out << "\"TimeId\": \"" << result[i].getTimeId() << "\"," << endl;
        out << "\"RoundAudience\": \"" << result[i].getRoundAudience() << "\"," << endl;
        out << "\"RoundExpression\": \"" << result[i].getRoundExpressionId() << "\"," << endl;
        out << "\"Score\": \"" << result[i].getScore() << "\"," << endl;
        out << "\"Expression\": \"" << result[i].getKsvm() << "\"," << endl;
        out << "\"Permalink\": \"" << result[i].getUsername() << "\"}";
        if (i == result.size() - 1) {
            out << endl;
        } else {
            out << ","<< endl;
        }
    }
    out << "]" << endl << "}" << endl;
}

void* Codebits::createType(string& type){
	//TODO
	cout << "New type requested: " << type << endl;
	
	if (type == "/search" || 
	    type == "/latests" || 
	    type == "/best" ||
	    type == "/scoreboard" || 
	    type == "/mybestsmile" ||
	    type == "9090")
		return new Codebits(type);
		
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}


void Codebits::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse &resp)
{
	if(method != "GET" && method != "PUT"){
    resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
    resp.send();
    return ;
  }
  if(method == "PUT" && type == "9090"){
    index(input, queryStrings);
    resp.send();
    return;
  }
  
  resp.setContentType("application/json");
	ostream& out = resp.send();
 
  vector<GameImage> results;
  
  out.flush();
  if (type == "/search")
    search(queryStrings, results);
  else if(type == "/latests")
    latests(queryStrings,results);
	else if(type == "/best")
    best(queryStrings,results);
	else if(type == "/scoreboard")
    scoreboard(queryStrings,results);
	else if(type == "/mybestsmile")
    mybestsmile(queryStrings, results);
  
  dump_json(results, out);
  out.flush();
}

void Codebits::index(istream& in, map<string, string> parameters)
{ 
  if(parameters.count("url") == 0){
    cout << "/index: error: Missing parameter 'url'" << endl;
    return ;
  }
  
  GameImage gm(parameters["url"]);
  //string str(static_cast<stringstream const&>(stringstream() << in.rdbuf()).str());
  //cout << str << endl;

  gm.deserialize(in);

  cout << "1" << endl;
  vector<float> gabor = gm.getGaborFace();
  face_gabor.addPoints(Matrix<float>(&gabor[0], 1, gabor.size()));
  last_face_gabor_id++;
  
  cout << "2" << endl;

  vector<float> hist = gm.getHistFace();
  face_hist.addPoints(Matrix<float>(&hist[0], 1, hist.size()));
  last_face_hist_id++;
  
  vector<float> recon = gm.getReconFace();
  face_recon.addPoints(Matrix<float>(&recon[0], 1, recon.size()));
  last_face_recon_id++;
  
  gabor = gm.getGaborShirt();
  shirt_gabor.addPoints(Matrix<float>(&gabor[0], 1, gabor.size()));
  last_shirt_gabor_id++;
  
  hist = gm.getHistShirt();
  shirt_hist.addPoints(Matrix<float>(&hist[0], 1, hist.size()));
  last_shirt_hist_id++;
  
  recon = gm.getReconShirt();
  shirt_recon.addPoints(Matrix<float>(&recon[0], 1, recon.size()));
  last_shirt_recon_id++;

  gm.setFlannId(last_face_gabor_id);
  
  gm.storeSQL();
  
}

Index<L2<float> >* Codebits::getIndex(int id){
  switch(id){
    case BY_EXPRESSION:
      return &face_gabor;
    case BY_FACE:
      return &face_recon;
    case BY_SHIRT:
      return &shirt_hist;
    default:
      return NULL;
  }
}

vector<float> Codebits::getVectorForIndex(int id, int media_id){
  GameImage gm("");
  
  if(!gm.loadSQL(media_id)){
    return vector<float>();
  }
  
  switch(id){
    case BY_EXPRESSION:
      return gm.getGaborFace();
    case BY_FACE:
      return gm.getReconFace();
    case BY_SHIRT:
      return gm.getHistShirt();
    default:
      return vector<float>();
  }
} 
  
void Codebits::search(map<string, string> parameters, vector<GameImage> &result)
{
  bool error = false;
  
  int id = check_int_val(parameters["id"], 0, numeric_limits<int>::max(), error);
  int criteria = check_int_val(parameters["criteria"], 0, 2, error);

  if (error) {
    cout << "/search: error: Invalid parameter 'id' or 'criteria'" << endl;
  } else {
  
    Index<L2<float> >* index = getIndex(criteria);
    vector<float> vector = getVectorForIndex(criteria, id);
    
    int nn = 10;
    Matrix<float> query(&vector[0], 1, vector.size());
    
    Matrix<int> indices(new int[nn], 1, nn);
    
    Matrix<float> dists(new float[nn], 1, nn);
    
    // do a knn search, using 128 checks
    int found = index->knnSearch(query, indices, dists, nn, flann::SearchParams(128));
    
    cout << "Unknown: " << found << endl;
    
    int j;
    for(j = 0; j < found; j++)
      cout << "indice: " << indices[0][j] << " dist: " << dists[0][j] << endl;
    
    int i;
    for(i = 0; i < found; i++){
      std::vector<std::string> pars;
      std::stringstream sstm;
      sstm << indices[0][i];
      pars.push_back(sstm.str());
      std::vector<GameImage> gms;
      GameImage::executeQuery(0, pars,gms);
      result.push_back(gms[0]);
    }
    
    cout << "/search: 'id'= " << id << " 'criteria'= " << criteria << " OK" << endl;
  }
  
}

void Codebits::latests(map<string, string> parameters, vector<GameImage> &result)
{
  vector<string> pars;
  GameImage::executeQuery(1, pars, result);
}

void Codebits::best(map<string, string> parameters, vector<GameImage> &result)
{
	
  //Error checking
  if(parameters.count("criteria") == 0){
    cout << "/best: error: Missing parameter 'criteria'" << endl;
  }
            
  bool error = false;
  
  int criteria = check_int_val(parameters["criteria"], 0, 7, error);
  
  if (error) {
    cout << "/best: error: Invalid parameter 'criteria'" << endl;
  } else {
  
    vector<string> pars;
    std::stringstream sstm;
    sstm << criteria;
    pars.push_back(sstm.str());
    GameImage::executeQuery(2, pars,result);

    cout << "/best: 'criteria'= " << criteria << " OK" << endl;
  }
}

void Codebits::scoreboard(map<string, string> parameters, vector<GameImage> &result)
{
  vector<string> pars;
  cout << "aaaaaaaaaaa" << endl;
  GameImage::executeQuery(3, pars, result);
  cout << "dddddddddddd" << result.size() << endl;
}

void Codebits::mybestsmile(map<string, string> parameters, vector<GameImage> &result)
{
	
  bool error = false;

  string username = check_string_val(parameters["user"], error);

  if (error) {
    cout << "/mybestsmile: error: Invalid parameter 'user'" << endl;
  } else {
  
    vector<string> pars;
    pars.push_back(username);
    GameImage::executeQuery(4, pars, result);

    cout << "/mybestsmile: 'user'= " << username << " OK" << endl;	
  }
  
}
