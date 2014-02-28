import json
import sys

json_data=open(sys.argv[1],'rb')
data = json.load(json_data)
json_data.close()

total = 0

for k in data["endpoints"]["indexer"]:
	curr = 1
	for key in k["params"]:
		if isinstance(k["params"][key], list):
			curr*=  len(k["params"][key])
	total+=curr
print total
