#include "RestRequestHandler.h"

const string FEATURES = "features";
const string URL = "url";

RestRequestHandler::RestRequestHandler()
{
    //controller = new DataModelController();
};

RestRequestHandler::~RestRequestHandler()
{
};

void RestRequestHandler::handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
{
    resp.setStatus(HTTPResponse::HTTP_OK);
    resp.setContentType("text/plain");

    string method = req.getMethod();

    Poco::URI uri(req.getURI());
    string endpoint = uri.getPath();
    string query = uri.getQuery();

    map<string,string> parameters = getParams(query);

    cout << "#########################################################" << endl;
    cout << "\t" << method << " @ " << endpoint << endl;
    cout << "\tQuery: " << query << endl;
    cout << "#########################################################" << endl;
    
    //endpoint.erase(0,1); //Erase the first /
    
    FactoryEndpoint* fact = FactoryEndpoint::getInstance();
    
    IEndpoint* endpt = (IEndpoint*)(fact->createType(endpoint));
    
    if(endpt == NULL){
      cout << "\t" << endpoint << " does not exist." << endl;
      resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
      resp.send();
      return ;
    }
    
    endpt->handleRequest(method, parameters, req.stream(), resp);
    
    /*vector<string> features = getFeatures(parameters[FEATURES]);
    cout << "1" << endl;
    controller->newRequest(method, endpoint, parameters[URL], features); 
    cout << "2" << endl;*/
    
};

map<string,string> RestRequestHandler::getParams(string params)
{
    string part1 = params;
    string part2 = params;
    map<string,string> result;

    while(part2.size() > 0){
        size_t tokenPos = part2.find('&');
        if(tokenPos != string::npos)
        {
            part1 = part2.substr(0,tokenPos);
            part2 = part2.substr(tokenPos+1,part2.size()-tokenPos-1);
        }
        else
        {
            part1 = part2;
            part2 = "";
        }
        size_t equalPos = part1.find("=");
        string key = part1.substr(0,equalPos);
        string value = part1.substr(equalPos+1,part1.size()-equalPos-1);
        result[key] = value;
    }

    return result;
}

/*vector<string> RestRequestHandler::getFeatures(string features){
    string part1 = features;
    string part2 = features;
    vector<string> result;

    size_t tokenPos = part2.find(";");
    while(tokenPos != string::npos){
        tokenPos = part2.find(";");
        part1 = part2.substr(0,tokenPos);
        part2 = part2.substr(tokenPos+1,part2.size()-tokenPos-1);
        result.push_back(part1);
        tokenPos = part2.find(";");
    }
    result.push_back(part2);
    return result;
}*/
