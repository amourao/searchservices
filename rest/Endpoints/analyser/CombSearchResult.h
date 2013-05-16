#pragma once

#include <iostream>
#include <sstream>
#include <queue>
#include <map>
#include <vector>
#include <cmath>
#include <cstdio>

#include "SearchResult.h"



 class  compareSearchResults{
     public:
     bool operator()(SearchResult& t1, SearchResult& t2){
        return t1.getScore() < t2.getScore();
     }
 };
 
class CombSearchResult
{
 
public:
	CombSearchResult();
	~CombSearchResult();

	static std::priority_queue<SearchResult,std::vector<SearchResult>,compareSearchResults> combineResultsList(std::vector<std::vector<SearchResult> > toCombine, std::string combinationFunction);
	
	static std::vector<SearchResult> combineOnList(std::vector<SearchResult> toCombine, std::string combinationFunction);
	
	static std::map<std::string, std::vector<SearchResult> > invertLists(std::vector<std::vector<SearchResult> > toCombine, std::string combinationFunction);
	static SearchResult combineResult(std::vector<SearchResult> toCombine, std::string combinationFunction);
	
};


