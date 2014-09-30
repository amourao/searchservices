#include "FileDownloader.h"

FileDownloader::FileDownloader(){
    baseFolder = DOWNLOAD_FOLDER;
}

FileDownloader::FileDownloader(std::string downloadFolder){
    baseFolder = downloadFolder;
}



FileDownloader::~FileDownloader(){}

string FileDownloader::getCurrentDir(){
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  return string(cwd);
}

int FileDownloader::getFile(std::string url, std::string location){
  CURL *curl;
  CURLcode res;

  cout << url << endl;
  url = StringTools::replaceAll(url," ", "%20");
  if (url == ""){
    return -3;
  }

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

    std::FILE* file = std::fopen( location.c_str(), "w" ) ;
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, file ) ;

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK){
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
      return -1;
    }
    /* always cleanup */
    curl_easy_cleanup(curl);
    fclose (file);
  } else {
    return -2;
  }
  curl_global_cleanup();
  return 0;
}

std::vector<std::string> FileDownloader::getFiles(std::string url){
    std::vector<std::string> results;
    std::vector<std::string> x = StringTools::split(url, ';');
    for (int i = 0; i < x.size(); i++)
        results.push_back(getFile(x[i]));
    return results;
}

std::string FileDownloader::getFile(std::string url, int& status){
  std::stringstream ss;
  std::string name;
  ss << baseFolder << StringTools::genRandom(RANDOM_NAME_SIZE) << "." << getExtension(url);
  name = ss.str();
  status = getFile(url,name);
  return name;
}

std::string FileDownloader::getFile(std::string url){
  std::stringstream ss;
  std::string name;
  ss << baseFolder << StringTools::genRandom(RANDOM_NAME_SIZE) << "." << getExtension(url);
  name = ss.str();
  getFile(url,name);
  return name;
}



std::string FileDownloader::getExtension(std::string name) {
    if(name.find_last_of(".") != std::string::npos)
        return name.substr(name.find_last_of(".")+1);
    return "";
}

bool FileDownloader::fileExists(std::string path){
    struct stat buffer;
    return (stat (path.c_str(), &buffer) == 0);
}

