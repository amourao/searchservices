#include "RestServer.h"

const int port = 9090;

RestServer* RestServer::instance = NULL;

RestServer::RestServer()
{
    HTTPServer s(new RequestHandlerFactory, ServerSocket(port), new HTTPServerParams);
    s.start();
    cout << endl << "Server started" << endl;

    waitForTerminationRequest(); 
};

RestServer::~RestServer()
{
};

HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest &)
{
    return new RestRequestHandler();
};

RestServer* RestServer::getInstance()
{
    if (instance == NULL)
        instance = new RestServer();
    return instance;
};

