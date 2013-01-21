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


void Codebits::handleRequest(string method, map<string, string> queryStrings, HTTPServerResponse &resp)
{
  if(method != "GET"){
    resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
    resp.send();
    return ;
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

string Codebits::search(map<string, string> parameters)
{
  string response("Reached Search!");
  return response;
}

string Codebits::latests(map<string, string> parameters)
{
	//TODO: db << "SELECT * FROM GameImages ORDER BY TimerId DESC", into(result), limit(50), now;
  
  string response("Reached Latests!");
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
  
  return response;
}

string Codebits::scoreboard(map<string, string> parameters)
{
  //TODO: db << "SELECT * FROM GameImages ORDER BY Score DESC", into(result), limit(50), now;

  string response("Reached Scoreboard!");
  return response;
}

string Codebits::mybestsmile(map<string, string> parameters)
{
  string response("Reached MyBestSmile!");
  return response;
}
