#include "StringTools.h"

std::string StringTools::genRandom(int len) {
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

std::string StringTools::replaceAll(string input, string search, string replace) {
    size_t pos = 0;
    while ((pos = input.find(search, pos)) != std::string::npos) {
         input.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return input;
}