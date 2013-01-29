#include "Codebits.h"

static Codebits codebitsFactory;
static KSVDFinder<KSVDIndex<OMPSparseConstrained, fmat>>& _index;

Codebits::Codebits(string type){
  this->type = type;
}

Codebits::Codebits(){
	FactoryEndpoint::getInstance()->registerType("/search",this);
	FactoryEndpoint::getInstance()->registerType("/latests",this);
	FactoryEndpoint::getInstance()->registerType("/best",this);
	FactoryEndpoint::getInstance()->registerType("/scoreboard",this);
	FactoryEndpoint::getInstance()->registerType("/mybestsmile",this);
	
	
    
  Poco::Util::AbstractConfiguration* config = new Poco::Util::XMLConfiguration("config.xml");
       
  bool all_ok = true;
  // Index configuration
  auto fdb = config->getString("features-db");
  kc::PolyDB feature_db;
  if (Poco::File(fdb).exists()) {
    all_ok &= feature_db.open(fdb, kc::PolyDB::OWRITER | kc::PolyDB::OREADER);
  } else {
    all_ok &= feature_db.open(fdb, kc::PolyDB::OWRITER | kc::PolyDB::OCREATE | kc::PolyDB::OREADER);            
  }
    
  auto qdb = config->getString("queries-db");
  kc::PolyDB queries_db;
  if (Poco::File(qdb).exists()) {
    all_ok &= queries_db.open(qdb, kc::PolyDB::OWRITER | kc::PolyDB::OREADER);
  } else {
    all_ok &= queries_db.open(qdb, kc::PolyDB::OWRITER | kc::PolyDB::OCREATE | kc::PolyDB::OREADER);            
  }
    
  int index_capacity = config->getInt("index-capacity");
    
  OMPSparseConstrained::Options opt;
  opt.max_iters = 10;
  opt.eps = 1e-7;
    
  OMPSparseConstrained omp(opt);
    
  typedef KSVDIndex<OMPSparseConstrained, fmat> index_type;
    
  vector<index_type *> indexes;
    
  fmat fgabor;
  all_ok &= fgabor.load(config->getString("face-gabor-dict"));
  index_type ifg(omp, fgabor, index_capacity);
  indexes.push_back(&ifg);
    
  fmat fhist;
  all_ok &= fhist.load(config->getString("face-hist-dict"));
  index_type ifh(omp, fhist, index_capacity);
  indexes.push_back(&ifh);
    
  fmat frecon;
  all_ok &= frecon.load(config->getString("face-recon-dict"));
  index_type ifr(omp, frecon, index_capacity);
  indexes.push_back(&ifr);
    
  fmat sgabor;
  all_ok &= sgabor.load(config->getString("shirt-gabor-dict"));
  index_type sfg(omp, sgabor, index_capacity);
  indexes.push_back(&sfg);
    
  fmat shist;
  all_ok &= shist.load(config->getString("shirt-hist-dict"));
  index_type sfh(omp, shist, index_capacity);
  indexes.push_back(&sfh);
    
  fmat srecon;
  all_ok &= srecon.load(config->getString("shirt-recon-dict"));
  index_type sfr(omp, srecon, index_capacity);
  indexes.push_back(&sfr);
    
  if (!all_ok) {
    std::cout << "upsy" << std::endl;
    return Poco::Util::Application::EXIT_CONFIG;
  }
    
  KSVDFinder<index_type> finder(queries_db, feature_db, indexes);
  finder.load();
  
  _index = finder;

  Poco::File f(sql_file);
  if (!f.exists() || (args.size() >= 2 && !args[1].compare("clean")))
  {
    Poco::Data::Session db(Poco::Data::SQLite::Connector::KEY, sql_file);
    db << "DROP TABLE IF EXISTS GameImages", now;
    db << "CREATE TABLE IF NOT EXISTS GameImages (Pk INTEGER, GameId INTEGER, RoundId INTEGER, UserId INTEGER, TimerId INTEGER, RoundAudience INTEGER, RoundExpressionId INTEGER, Score REAL, Ksvm INTEGER, UserName VARCHAR)", now;
    db.close();
	}
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
void dump_json(const std::vector<GameImage>& result, std::ostream& out) {
    out << "{ \"rank\": [" << endl;
    for (size_t i = 0; i != result.size(); i++) {
        out << "{\"pk\": \"" << result[i].PK << "\"," << endl;
        out << "\"GameId\": \"" << result[i].GAME_ID << "\"," << endl;
        out << "\"RoundId\": \"" << result[i].ROUND_ID << "\"," << endl;
        out << "\"UserId\": \"" << result[i].USER_ID << "\"," << endl;
        out << "\"TimeId\": \"" << result[i].TIME_ID << "\"," << endl;
        out << "\"RoundAudience\": \"" << result[i].ROUND_AUDIENCE << "\"," << endl;
        out << "\"RoundExpression\": \"" << result[i].ROUND_EXPRESS_ID << "\"," << endl;
        out << "\"Score\": \"" << int(result[i].SCORE * 100) << "\"," << endl;
        out << "\"Expression\": \"" << result[i].KSVM << "\"," << endl;
        out << "\"Permalink\": \"" << result[i].USERNAME << "\"}";
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
 
  string response("");
  
  if (type == "/search")
    response = search(queryStrings);
  else if(type == "/latests")
    response = latests(queryStrings);
	else if(type == "/best")
    response = best(queryStrings);
	else if(type == "/scoreboard")
    response = scoreboard(queryStrings);
	else if(type == "/mybestsmile")
    response = mybestsmile(queryStrings);
  
  out << response;
  out.flush();
}

//TODO: Check if all parameters are present
void index(istream& in)
{
  //TODO: Add new gameImage
  bool error = false;
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
  
  gm.storeSQL();
}

string Codebits::search(map<string, string> parameters)
{
  string response("Reached Search!");
  
	vector<GameImage> result;
  bool error = false;
  
  auto id = static_cast<Finder::id_type>(check_int_val(parameters["id"], 0, numeric_limits<int>::max(), error));
  auto criteria = static_cast<Finder::criteria_type>(check_int_val(parameters["criteria"], 0, 2, error));

  if (error) {
    response = "{\"error\": \"Invalid parameter 'id' or 'criteria'\"}";
  } else {
    vector<Finder::id_type> rank = _index.find(id, criteria);
    if (!rank.empty()) {
      db << "SELECT * FROM GameImages where PK == :rk", use(rank), into(result), now;                        
    }
    cout << "/search: 'id'= " << id << " 'criteria'= " << criteria << " OK" << endl;
  }
  
  return response;
}

string Codebits::latests(map<string, string> parameters)
{
  string response("Reached Latests!");
	
	db << "SELECT * FROM GameImages ORDER BY TimerId DESC", into(result), limit(50), now;
  return response;
}

string Codebits::best(map<string, string> parameters)
{
  string response("Reached Best!");
  
  //Error checking
  if(parameters.count("criteria") == 0){
    response = "{\"error\": \"Missing parameter 'criteria'\"}";
    return response;
  }
            
  bool error = false;
  
  auto criteria = static_cast<Finder::criteria_type>(check_int_val(parameters["criteria"], 0, 7, error));
  
  if (error) {
    response = "{\"error\": \"Invalid parameter 'criteria'\"}";
  } else {
    db << "SELECT * FROM GameImages WHERE RoundExpressionId == :criteria ORDER BY Score DESC", use(criteria), into(result), limit(50), now;	
    cout << "/best: 'criteria'= " << criteria << " OK" << endl;
  }
    
  return response;
}

string Codebits::scoreboard(map<string, string> parameters)
{
  string response("Reached Scoreboard!");
  
  db << "SELECT * FROM GameImages ORDER BY Score DESC", into(result), limit(50), now;
  return response;
}

string Codebits::mybestsmile(map<string, string> parameters)
{
  string response("Reached MyBestSmile!");
  
  bool error = false;

  string username = check_string_val(parameters["user"], error);

  if (error) {
    response = "{\"error\": \"Invalid parameter 'user'\"}";
  } else {
    db << "SELECT * FROM GameImages WHERE UserName == :username AND Ksvm == 5 ORDER BY Score DESC", use(username), into(result), limit(1), now;
    cout << "/mybestsmile: OK" << endl;	
  }
  
  return response;
}
