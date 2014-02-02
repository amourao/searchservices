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

};

