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


#define DOWNLOAD_FOLDER "./tmpData/"
#define RANDOM_NAME_SIZE 32

class FileDownloader {

public:
	FileDownloader();
	~FileDownloader();
	
    void getFile(std::string url, std::string location);
    
    std::string getFile(std::string url);
    std::vector<std::string> getFiles(std::string url);
    
    
	
private:
    std::string getExtension(std::string name);
    std::string genRandom(int len);
    
    std::vector<std::string> split(const std::string &s, char delim);
    
    std::string type;
};
