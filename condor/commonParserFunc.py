import json
import csv	
import sys
import copy
import ast
import numpy

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

def get_params(jsonDict):
	listOflists = []

	indexers = jsonDict['endpoints']['indexer']
	curr = 1
	total = 0
	for k in indexers:
		combinations = []
		newCombinations = []
		mapIntName = []

		lists = []
		for key in k['params']:
			mapIntName.append(key)
			if isinstance(k['params'][key], list):
				lists.append(k['params'][key])
			else:
				lists.append([k['params'][key]])

		for value in lists[0]:
			inner = {}
			inner[mapIntName[0]] = value
			combinations.append(inner)
		i = 1
		for next in lists[1:]:
			newCombinations = []
			j = 0
			for it in combinations:
				for it2 in next:
					it[mapIntName[i]] = it2
					newDict = copy.deepcopy(it)
					newCombinations.append(newDict)
			combinations = newCombinations[:]
			i+=1
		listOflists.extend(combinations)

	return listOflists


def count_total(data):
	total = 0
	for k in data["endpoints"]["indexer"]:
		curr = 1
		for key in k["params"]:
			if isinstance(k["params"][key], list):
				curr*=  len(k["params"][key])
		total+=curr
	print total

def paramsInList(paramList,filterList):
	for param in filterList:
		for key in param:
			if key in paramList:
				if not (paramList[key] in param[key]):
					return True
	return False

def paramsNotInList(paramList,filterList):
	for param in filterList:
		for key in param:
			if key in paramList:
				if (paramList[key] in param[key]):
					return True
	return False
