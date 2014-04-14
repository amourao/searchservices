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


std::vector<std::string> StringTools::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
