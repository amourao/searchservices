import json
import csv	
import sys


def is_number(s):
    try:
        float(s)
        return True
    except ValueError:
        return False

def main():

	listOflistsAccum = []

	headers = []

	pre_collumns = []

	fi = 0

	numsStartAt = 0

	skipHeaders = 4

	accum = len(sys.argv)-1
	for fileName in sys.argv[1:]:
		f = open(fileName,'rb')

		reader=csv.reader(f,delimiter=';')

		i = 0

		for row in reader:
			j = 0
			if fi == 0:
				if i < skipHeaders:
					print ';'.join(row)
			realCol = 0
			if i >= skipHeaders:
				for col in row:
					realRow = i-skipHeaders
					if (not is_number(col) or j == 0) and fi == 0:
						if len(pre_collumns) <= (realRow):
							pre_collumns.append([])
						pre_collumns[realRow].append(col)
						numsStartAt = max(numsStartAt,j)
						
					elif is_number(col) and j != 0:
						realCol = j-numsStartAt-1
						if len(listOflistsAccum) <= (realRow):
							listOflistsAccum.append([])
						if len(listOflistsAccum[realRow]) <= (realCol):
							listOflistsAccum[realRow].append(0.0)
						listOflistsAccum[realRow][realCol]+=float(col)
					j+=1
			i+=1

		fi+=1

	row = 0
	col = 0
	for val in listOflistsAccum:
		print ';'.join(pre_collumns[row]) + ';',
		for val1 in val:
			sys.stdout.write((str(val1/accum) + ';'))
		print ''
		row+=1

if __name__ == "__main__":
	main()