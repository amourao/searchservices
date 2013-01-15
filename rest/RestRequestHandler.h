#include "InterfaceController.h"
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/ServerApplication.h>
#include "Poco/URI.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace Poco::Net;
using namespace Poco::Util;
using namespace std;

class RestRequestHandler : public HTTPRequestHandler
{                                                                        
    public:
        RestRequestHandler();
        ~RestRequestHandler();
        virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp);

    private:
        DataModelController* controller;

        map<string,string> getParams(string params);
        vector<string> getFeatures(string features);
};
