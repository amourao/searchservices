#include "AnalyserDataType.h"

AnalyserDataType::AnalyserDataType(void* _data, string _name){
	data = _data;
	name = _name;
}

AnalyserDataType::~AnalyserDataType(){
	
}
	
void* AnalyserDataType::getData(){
	return data;	
}
	
string AnalyserDataType::getName(){
	return name;
}
