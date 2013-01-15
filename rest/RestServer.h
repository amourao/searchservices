#include "RestRequestHandler.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/ServerApplication.h>

using namespace Poco::Net;
using namespace Poco::Util;
using namespace std;

class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
    public:
        virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &);

};

class RestServer : public ServerApplication
{
    public:
        static RestServer* getInstance();

        ~RestInterface();

    private:
        static RestServer* instance;

        RestRestServer();

};
