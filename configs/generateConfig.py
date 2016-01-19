#!/usr/bin/python
import sys

portServer=12350
nServers=2
portProcessors=12360
nBuckets=1024
processors1=['arthur','cobb','mal','saito']
processors=['localhost']
processorsPerServer=2
divisionsPerProcessor=2
nQueries=100
bufferOffset=0
divisions=nServers
limit=0.1
n=10
path='tmp'


data="/localstore/amourao/saIndexingSplits/configCondor4_3.json_testI.mat",    
dataQ="/localstore/amourao/saIndexingSplits/configCondor4_3.json_testQ.mat"

servers=''
for processor in processors:
	for port in range(portProcessors,portProcessors+processorsPerServer):
		servers += processor + ':' + str(port) + ';'
servers=servers[:-1]


for i in range(nServers):
	f = open('defaultMaster_' + str(i) + '.json', 'w')
	sys.stdout = f
	print '{"parameters": {'
	print '\t"extractor": "OMP",'
	print '\t"bufferSize": "65536",'
	print '\t"limit": "'+str(limit)+'",'
	print '\t"n": "'+str(n)+'",'
	print '\t"servers": "'+servers+'",'
	print '\t"dataQ": "'+dataQ+'",'
	print '\t"nQueries": "'+str(nQueries)+'",'
	print '\t"port": "'+str(portServer)+'"'


	portServer+=1
	print '},'
	print '\t"endpoints": {'
	print '\t\t"indexer": [],'
	print '\t\t"analyser": ['
	print '\t\t\t{'
	print '\t\t\t\t"newName": "OMP",'
	print '\t\t\t\t"originalName": "ANdOMPExtractor",'
	print '\t\t\t\t"params": {'
	print '\t\t\t\t\t"max_iters": "10",'
	print '\t\t\t\t\t"eps": "0.05",'
	print '\t\t\t\t\t"dictPath": "/home/amourao/code/searchservices/workingDir/seed.bin"'
	print '\t\t\t\t}'
	print '\t\t\t}'
	print '\t\t],'
	print '\t\t"classifier": []'
	print '\t}'
	print '}'

offset = 0
nBucketsPerServer = nBuckets/len(processors)/processorsPerServer

for processor in processors:
	i=0
	for port in range(portProcessors,portProcessors+processorsPerServer):
		f = open('defaultProcessor_' + processor + "_" + str(i) + '.json', 'w')
		sys.stdout = f
		i+=1
		print '{"parameters": {'
		print '\t"bufferSize": "65536",'
		print '\t"totalBuckets": "'+str(nBuckets)+'",'
		print '\t"bucketCount": "'+str(nBucketsPerServer)+'",'
		print '\t"bucketOffset": "'+str(offset)+'",'
		print '\t"path": "'+path+'",'
		offset+=nBucketsPerServer
		print '\t"port": "'+str(port)+'",'
		print '\t"divisions": "'+str(divisionsPerProcessor)+'"'
		print '},'
		print '\t"endpoints": {'
		print '\t\t"indexer": [],'
		print '\t\t"analyser": [],'
		print '\t\t"classifier": []'
		print '\t}'
		print '}'

