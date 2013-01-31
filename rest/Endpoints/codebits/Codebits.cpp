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
	
  face_gabor.buildIndex();
  /*face_hist.buildIndex();
  face_recon.buildIndex();
  shirt_gabor.buildIndex();
  shirt_hist.buildIndex();
  shirt_recon.buildIndex();*/
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

unsigned int pk(int game_id, int round_id, int user_id) {
    return ((game_id & 0x00007FFF) << 17) + ((round_id & 0x00000007) << 14) + (user_id & 0x00003FFF);
}

//bool starts_with(const std::string& str, const std::string& pre) {
//    return str.find(pre) == 0;
//}

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
        out << "\"RoundId\": \"" << result[i].getRoundId() << "\"," << endl;
        out << "\"UserId\": \"" << result[i].getUserId() << "\"," << endl;
        out << "\"TimeId\": \"" << result[i].getTimeId() << "\"," << endl;
        out << "\"RoundAudience\": \"" << result[i].getRoundAudience() << "\"," << endl;
        out << "\"RoundExpression\": \"" << result[i].getRoundExpressionId() << "\"," << endl;
        out << "\"Score\": \"" << int(result[i].getScore() * 100) << "\"," << endl;
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
	    type == "/mybestsmile")
		return new Codebits(type);
		
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}


void Codebits::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse &resp)
{
  if(method != "GET" && method != "POST"){
    resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
    resp.send();
    return ;
  }
  
  if(method == "POST"){
    index(input);
    return;
  }
  
  resp.setContentType("application/json");
	ostream& out = resp.send();
 
  vector<GameImage> results;
  
  if (type == "/search")
    results = search(queryStrings);
  else if(type == "/latests")
    results = latests(queryStrings);
	else if(type == "/best")
    results = best(queryStrings);
	else if(type == "/scoreboard")
    results = scoreboard(queryStrings);
	else if(type == "/mybestsmile")
    results = mybestsmile(queryStrings);
  
  dump_json(results, out);
  out.flush();
}

//TODO: Check if all parameters are present
void Codebits::index(istream& in)
{
  //TODO: Add new gameImage
  /*bool error = false;
  bool all_ok = true;
  
  int game_id = to_int(parameters["game_id"], error);
  all_ok &= error;
  int round_id = to_int(parameters["round_id"], error);
  all_ok &= error;
  int user_id = to_int(parameters["user_id"], error);
  all_ok &= error;
  int time_id = to_int(parameters["time_id"], error);
  all_ok &= error;
  int round_audience = to_int(parameters["round_audience"], error);
  all_ok &= error;
  int round_expr_id = to_int(parameters["round_expr_id"], error);
  all_ok &= error;
  int ksvm = to_int(parameters["ksvm"], error);
  all_ok &= error;
  istringstream stm(parameters["score"]);
  float score;
  stm >> score;
  string username = parameters["username"]

  if(!all_ok)
    return;
  
  unsigned int pk = pk(game_id, round_id, user_id);
  GameImage gm(pk, 
               game_id,
               round_id,
               user_id,
               time_id,
               round_audience,
               round_expr_id,
               ksvm,
               score,
               username);
  
  // Store features in HIndex
  vector<Finder::feature_vector> fdata;
  fdata.push_back(gm.FACE_GABOR);
  fdata.push_back(gm.FACE_HIST);
  fdata.push_back(gm.FACE_RECON);
  fdata.push_back(gm.SHIRT_GABOR);
  fdata.push_back(gm.SHIRT_HIST);
  fdata.push_back(gm.SHIRT_RECON);

  _index.insert(gm.PK, fdata, (Finder::expression_type) gm.KSVM);
  
  gm.storeSQL();*/
}

vector<GameImage> Codebits::search(map<string, string> parameters)
{
	vector<GameImage> result;
  bool error = false;
  
  int id = check_int_val(parameters["id"], 0, numeric_limits<int>::max(), error);
  int criteria = check_int_val(parameters["criteria"], 0, 2, error);

  if (error) {
    cout << "/search: error: Invalid parameter 'id' or 'criteria'" << endl;
  } else {
    // do a knn search, using 128 checks
    //index.knnSearch(query, indices, dists, nn, flann::SearchParams(128));
    
    //vector<Finder::id_type> rank = _index.find(id, criteria);
    //if (!rank.empty()) {
      //db << "SELECT * FROM GameImages where PK == :rk", use(rank), into(result), now;                        
    //}
    cout << "/search: 'id'= " << id << " 'criteria'= " << criteria << " OK" << endl;
  }
  
  return result;
}

vector<GameImage> Codebits::latests(map<string, string> parameters)
{
  vector<GameImage> result;
	
	//db << "SELECT * FROM GameImages ORDER BY TimerId DESC", into(result), limit(50), now;
  return result;
}

vector<GameImage> Codebits::best(map<string, string> parameters)
{
  vector<GameImage> result;
	
  //Error checking
  if(parameters.count("criteria") == 0){
    cout << "/best: error: Missing parameter 'criteria'" << endl;
    return result;
  }
            
  bool error = false;
  
  int criteria = check_int_val(parameters["criteria"], 0, 7, error);
  
  if (error) {
    cout << "/best: error: Invalid parameter 'criteria'" << endl;
  } else {
    //db << "SELECT * FROM GameImages WHERE RoundExpressionId == :criteria ORDER BY Score DESC", use(criteria), into(result), limit(50), now;	
    cout << "/best: 'criteria'= " << criteria << " OK" << endl;
  }
    
  return result;
}

vector<GameImage> Codebits::scoreboard(map<string, string> parameters)
{
  vector<GameImage> result;
	 
  //db << "SELECT * FROM GameImages ORDER BY Score DESC", into(result), limit(50), now;
  return result;
}

vector<GameImage> Codebits::mybestsmile(map<string, string> parameters)
{
  vector<GameImage> result;
	
  bool error = false;

  string username = check_string_val(parameters["user"], error);

  if (error) {
    cout << "/mybestsmile: error: Invalid parameter 'user'" << endl;
  } else {
    //db << "SELECT * FROM GameImages WHERE UserName == :username AND Ksvm == 5 ORDER BY Score DESC", use(username), into(result), limit(1), now;
    cout << "/mybestsmile: 'user'= " << username << " OK" << endl;	
  }
  
  return result;
}
