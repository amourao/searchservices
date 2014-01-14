#include "FileDownloader.h"

	FileDownloader::FileDownloader(){
    srand (time(NULL));
    }
	FileDownloader::~FileDownloader(){}

void FileDownloader::getFile(std::string url, std::string location){
  CURL *curl;
  CURLcode res;
  
  url = StringTools::replaceAll(url," ", "%20");

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    std::FILE* file = std::fopen( location.c_str(), "w" ) ;
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, file ) ;

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
    fclose (file);
  }
}

std::vector<std::string> FileDownloader::getFiles(std::string url){
    std::vector<std::string> results;
    std::vector<std::string> x = split(url, ';');
    for (int i = 0; i < x.size(); i++)
        results.push_back(getFile(x[i]));
    return results;
}

std::string FileDownloader::getFile(std::string url){
  std::stringstream ss;
  std::string name;
  ss << DOWNLOAD_FOLDER << StringTools::genRandom(RANDOM_NAME_SIZE) << "." << getExtension(url);
  name = ss.str();
  getFile(url,name);
  return name;
}




std::string FileDownloader::getExtension(std::string name) {
    if(name.find_last_of(".") != std::string::npos)
        return name.substr(name.find_last_of(".")+1);
    return "";
}

std::vector<std::string> FileDownloader::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
