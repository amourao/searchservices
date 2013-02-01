#include "RestServer.h"

RestServer::RestServer(int port)
{
    try {
      HTTPServer s(new RequestHandlerFactory, ServerSocket(port), new HTTPServerParams);
      s.start();
      cout << "Server started" << endl;
      
      //DataModelController dmc;
      
      waitForTerminationRequest();
      
      s.stop();
	    cout << endl << "Shutting down..." << endl;
    } catch (Poco::Net::NetException e){
      cout << endl << "Error while starting server: " << e.message() << endl;
    } 
};

RestServer::~RestServer()
{
};

HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest &)
{
    return new RestRequestHandler();
};
