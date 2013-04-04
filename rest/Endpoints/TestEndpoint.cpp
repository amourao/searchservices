#include "TestEndpoint.h"

static TestEndpoint testEndpointFactory;

TestEndpoint::TestEndpoint(string type){
  this->type = type;
}

TestEndpoint::TestEndpoint(){

	FactoryEndpoint::getInstance()->registerType("Hello",this);

}

TestEndpoint::~TestEndpoint()
{}

void* TestEndpoint::createType(string& type){
	//TODO
	cout << "New type requested: " << type << endl;
	
	if (type == "/Hello")
		return new TestEndpoint(type);
		
	cerr << "Error registering type from constructor (this should never happen)" << endl;
	return NULL;
}


void TestEndpoint::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  resp.setContentType("text/html");

	ostream& out = resp.send();
  
  out << "<h2>Hello World!</h2>";

}

