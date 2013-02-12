#include "TestEndpoint.h"

static TestEndpoint testEndpointFactory;

TestEndpoint::TestEndpoint(string type){
  this->type = type;
}

TestEndpoint::TestEndpoint(){
<<<<<<< HEAD
	FactoryEndpoint::getInstance()->registerType("/Hello",this);
=======
	FactoryEndpoint::getInstance()->registerType("Hello",this);
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d
}

TestEndpoint::~TestEndpoint()
{}

void* TestEndpoint::createType(string& type){
	//TODO
	cout << "New type requested: " << type << endl;
	
<<<<<<< HEAD
	if (type == "/Hello")
=======
	if (type == "Hello")
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d
		return new TestEndpoint(type);
		
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}


<<<<<<< HEAD
void TestEndpoint::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  resp.setContentType("text/html");

	ostream& out = resp.send();
  
  out << "<h2>Hello World!</h2>";
=======
void TestEndpoint::handleRequest(string method, map<string, string> queryStrings, HTTPServerResponse &resp)
{
  resp.setContentType("text/html");
  resp.send().write("<h2>Hello World!</h2>", strlen("<h2>Hello World!</h2>"));
>>>>>>> 4c67f65989eaf009ed2f9b45b6c33a0fbf1a911d
}

