#include "CombSearchResult.h"

CombSearchResult::CombSearchResult(){}

CombSearchResult::~CombSearchResult(){}

std::priority_queue<SearchResult,std::vector<SearchResult>,compareSearchResults> CombSearchResult::combineResultsList(std::vector<std::vector<SearchResult> > toCombine, std::string combinationFunction){
	std::map<std::string,std::vector<SearchResult> >	invertedList = CombSearchResult::invertLists(toCombine, combinationFunction);
	std::priority_queue<SearchResult,std::vector<SearchResult>,compareSearchResults> compressedList;
	
	std::map<std::string,std::vector<SearchResult> >::iterator iter;
    
    int rank = 1;
    for (iter = invertedList.begin(); iter != invertedList.end(); ++iter){
		SearchResult sr = CombSearchResult::combineResult(iter->second,combinationFunction);
		sr.setRank(rank++);
        compressedList.push(sr);
	 }
	return compressedList; 
}

std::map<std::string,std::vector<SearchResult> > CombSearchResult::invertLists(std::vector<std::vector<SearchResult> > toCombine, std::string combinationFunction){
	std::map<std::string,std::vector<SearchResult> >	results;
	for(unsigned int i = 0; i < toCombine.size(); i++){
		std::vector<SearchResult> resultList = CombSearchResult::combineOnList(toCombine.at(i), combinationFunction);
		for(unsigned int j = 0; j < resultList.size(); j++){
			SearchResult result = resultList.at(j);
			//if (results.find(result.getId()) == results.end())
			//	results[result.getId()];
			results[result.getId()].push_back(result);
		}
	}
	
	return results;
}

SearchResult CombSearchResult::combineResult(std::vector<SearchResult> toCombine, std::string combinationFunction){
	double newScore = 0;
	std::string idz = toCombine.at(0).getId();
	std::string doi = toCombine.at(0).getDOI();
	std::string iri = toCombine.at(0).getIRI();
	int rank = 999999;
	for(unsigned int i = 0; i < toCombine.size(); i++){
		if (rank > toCombine.at(i).getRank())
			rank = toCombine.at(i).getRank();
		
		if (combinationFunction == "combfct")
			newScore += 1.0/pow(toCombine.at(i).getRank(),2);
		else if (combinationFunction == "rrf")
			newScore += 1.0/(toCombine.at(i).getRank()+60);
		else if (combinationFunction == "combmnz")
			newScore += toCombine.at(i).getScore();
		else if (combinationFunction == "combmax")
			if (newScore < toCombine.at(i).getScore())
				newScore = toCombine.at(i).getScore();
	}
	
	
	if (combinationFunction == "combfct")
		newScore *= log(toCombine.size());
	else if (combinationFunction == "combmnz")
		newScore *= toCombine.size();
	else if (combinationFunction == "rrf")
		;
	else if (combinationFunction == "combmax")
		;
	
	//std::cout << newScore << std::endl;
	return SearchResult(idz,newScore,rank,iri,doi);
}

std::vector<SearchResult> CombSearchResult::combineOnList(std::vector<SearchResult> toCombine, std::string combinationFunction){
	std::vector<SearchResult> results;
	std::map<std::string,std::vector<SearchResult> > results2;
	
	for(unsigned int j = 0; j < toCombine.size(); j++){
		//std::cout << "a " << toCombine.at(j).getId() << std::endl;
		results2[toCombine.at(j).getId()].push_back(toCombine.at(j));
	}
	
	std::map<std::string,std::vector<SearchResult> >::iterator iter;
    //std::cout << "b " << results2.size() << std::endl;
	for (iter = results2.begin(); iter != results2.end(); ++iter)
		results.push_back(CombSearchResult::combineResult(iter->second,combinationFunction));
           	
	return results;
}
