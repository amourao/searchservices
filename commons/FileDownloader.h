#pragma once

#include <curl/curl.h>
#include <string>
#include <cstdio>
#include <vector>
#include <sstream>
#include <iostream>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <unistd.h>
#include "StringTools.h"

#define DOWNLOAD_FOLDER "/localstore/amourao/code/searchservices/tmpData/"
#define RANDOM_NAME_SIZE 32

class FileDownloader {

public:
	FileDownloader();
	~FileDownloader();
	
    void getFile(std::string url, std::string location);
    
    std::string getFile(std::string url);
    std::vector<std::string> getFiles(std::string url);

    string getCurrentDir();
    
    
	
private:
    std::string getExtension(std::string name);
        
    std::string type;
};
