import json
import csv	
import sys
import copy
import ast
import numpy
import commonParserFunc

def main():

	xIndex = 5
	yIndex = 6

	json_data=open(sys.argv[1]).read()
	json_dict = json.loads(json_data);

	numsStartAt = 0

	skipHeaders = 4

	f = open(sys.argv[2],'rb')

	whiteList = ast.literal_eval(sys.argv[3])
	blackList = ast.literal_eval(sys.argv[4])

	reader=csv.reader(f,delimiter=';')

	listOfParams = commonParserFunc.get_params(json_dict)

	i = 0
	for row in reader:
		j = 0
		if i >= skipHeaders:
			mapValue = float(row[yIndex])
			time = float(row[xIndex])
			inWhiteList = not commonParserFunc.paramsInList(listOfParams[i-skipHeaders],whiteList)
			inBlackList = commonParserFunc.paramsNotInList(listOfParams[i-skipHeaders],blackList)
			if inWhiteList and not inBlackList:
				print ';'.join(row) + ';',
				for key in listOfParams[i-skipHeaders]:
					print key + ':' +  listOfParams[i-skipHeaders][key] + ';',
				print ''
		i+=1

if __name__ == "__main__":
	main()