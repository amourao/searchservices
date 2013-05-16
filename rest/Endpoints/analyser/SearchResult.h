#pragma once

#include <iostream>
#include <sstream>
 
class SearchResult
{
 
public:
	SearchResult(std::string id, double score, int rank, std::string iri, std::string doi);
	~SearchResult();
	
	
	std::string getId(); 
	std::string getIRI();
	std::string getDOI();
	double getScore(); 
	int getRank();
	void setRank(int rank);
	
private:
	

	std::string id; 
	double score; 
	int rank;
	std::string iri;
	std::string doi;
};

