import json
import csv	
import sys
import copy
import ast
import numpy
import commonParserFunc

class prettyfloat(float):
    def __repr__(self):        return "%0.3f" % self
    def __str__(self):
         return "%0.3f" % self

class prettyint(int):
    def __repr__(self):        return "%03d" % self
    def __str__(self):
         return "%03d" % self

def is_number(s):
	try:
		float(s)
		return True
	except ValueError:
		return False

def main():

	xIndex = 5
	yIndex = 6

	json_data=open(sys.argv[1]).read()
	json_dict = json.loads(json_data);

	numsStartAt = 0

	skipHeaders = 4

	f = open(sys.argv[2],'rb')

	reader=csv.reader(f,delimiter=';')

	listOfParams = commonParserFunc.get_params(json_dict)

	varyingmAcc = {}
	varyingTime = {}

	i = 0
	for row in reader:
		j = 0
		if i >= skipHeaders:
			mapValue = float(row[yIndex])
			time = float(row[xIndex])
			for key in listOfParams[i-skipHeaders]:
				if not (key in varyingmAcc):
					varyingmAcc[key] = {}
					varyingTime[key] = {}
				if not (listOfParams[i-skipHeaders][key] in varyingmAcc[key]):
					varyingmAcc[key][listOfParams[i-skipHeaders][key]] = []
					varyingTime[key][listOfParams[i-skipHeaders][key]] = []
				varyingmAcc[key][listOfParams[i-skipHeaders][key]].append(mapValue)
				varyingTime[key][listOfParams[i-skipHeaders][key]].append(time)

		i+=1

	for key in varyingmAcc:
		print key
		for key2 in sorted(varyingmAcc[key]):
			if (len(key2) > 6):
				print key2 + '\t',
			else:
				print key2 + '\t\t',
			print prettyfloat(numpy.mean(varyingmAcc[key][key2])),
			print prettyfloat(numpy.std(varyingmAcc[key][key2])),
			print prettyfloat(numpy.mean(varyingTime[key][key2])),
			print prettyfloat(numpy.std(varyingTime[key][key2]))

if __name__ == "__main__":
	main()