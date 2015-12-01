#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>

using namespace std;

class StringTools {

public:

    static std::string genRandom(int len);
    static std::string replaceAll(string input, string search, string replace);
    static std::vector<std::string> split(const std::string &s, char delim);
    static bool endsWith(std::string value, std::string ending);

};

