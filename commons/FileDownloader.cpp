#include "FileDownloader.h"

	FileDownloader::FileDownloader(){
    srand (time(NULL));
    }
	FileDownloader::~FileDownloader(){}

void FileDownloader::getFile(std::string url, std::string location){
  CURL *curl;
  CURLcode res;

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

std::string FileDownloader::getFile(std::string url){
  std::stringstream ss;
  std::string name;
  ss << DOWNLOAD_FOLDER << genRandom(RANDOM_NAME_SIZE) << "." << getExtension(url);
  name = ss.str();
  getFile(url,name);
  return name;
}

std::string FileDownloader::genRandom(int len) {
    std::stringstream ss;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        ss << alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return ss.str();
}


std::string FileDownloader::getExtension(std::string name) {
    if(name.find_last_of(".") != std::string::npos)
        return name.substr(name.find_last_of(".")+1);
    return "";
}
