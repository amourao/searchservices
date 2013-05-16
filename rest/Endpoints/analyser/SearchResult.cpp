#include "SearchResult.h"

SearchResult::SearchResult(std::string _id, double _score, int _rank, std::string _iri, std::string _doi){
	id = _id;
	score = _score;
	rank = _rank;
	iri = _iri;
	doi = _doi;
}

SearchResult::~SearchResult(){}

std::string SearchResult::getId(){
	return id;
}

std::string SearchResult::getIRI(){
	return iri;
}

std::string SearchResult::getDOI(){
	return doi;
}

double SearchResult::getScore(){
	return score;
}

int SearchResult::getRank(){
	return rank;
}

void SearchResult::setRank(int _rank){
	rank = _rank;
}
