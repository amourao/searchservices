#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>
#include <cstdlib>
#include <time.h>
#include <armadillo>

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>

#include "tools/TrainTestFeaturesTools.h"
#include "tools/MIRFlickrImporter.h"
#include "tools/tinyImageImporter.h"
#include "tools/oneBillionImporter.h"
#include "tools/oneBillionImporterB.h"
#include "tools/IBinImporter.h"

#include "../indexer/distributed/SRStrucs.h"
#include "../indexer/distributed/SRProcessor.h"
#include "../indexer/distributed/SRMaster.h"

#include "../commons/StringTools.h"
#include "../commons/Timing.h"
#include "../commons/LoadConfig.h"

using namespace std;

int testDistIndexer(int argc, char *argv[]){

    QueryStructReq s;
    s.buckets.push_back(3);
    s.buckets.push_back(1);
    s.buckets.push_back(2);

    s.query.push_back(2.12);
    s.query.push_back(423.4223);
    s.query.push_back(123.12);
    s.query.push_back(5623.23);

    s.coeffs.push_back(123.123);
    s.coeffs.push_back(125.214);
    s.coeffs.push_back(3463.34634);

    s.indexes.push_back(4294967294);
    s.indexes.push_back(4294962324);
    s.indexes.push_back(4294);

    s.parameters.push_back(3463.34634);
    s.parameters.push_back(10);
    s.parameters.push_back(14);

    char* by = s.toBytes();
    cout << *reinterpret_cast<uint*>(&by[0]) << endl;
    cout << s.totalByteSize << endl;
    cout << s.computeTotalByteSize() << endl;


    QueryStructReq s2;
    s2.toQueryStructReq(by);
    delete[] by;

    cout << s2.buckets.size() << endl;
    cout << s2.query.size() << endl;
    cout << s2.coeffs.size() << endl;
    cout << s2.indexes.size() << endl;
    cout << s2.parameters.size() << endl;

    cout << s2.buckets[0] << " " << s2.buckets[1] << endl;
    cout << s2.query[0] << " " << s2.query[1] << endl;
    cout << s2.coeffs[0] << " " << s2.coeffs[1] << endl;
    cout << s2.indexes[0] << " " << s2.indexes[1] << endl;
    cout << s2.parameters[0] << " " << s2.parameters[1] << endl;

    QueryStructRsp s3;
    s3.indexes.push_back(4294967294);
    s3.indexes.push_back(4294962324);
    s3.indexes.push_back(4294);

    s3.dists.push_back(2.12);
    s3.dists.push_back(423.4223);
    s3.dists.push_back(123.12);
    s3.dists.push_back(5623.23);

    s3.parameters.push_back(3463.34634);
    s3.parameters.push_back(10);
    s3.parameters.push_back(14);

    char* by2 = s3.toBytes();

    QueryStructRsp s4;
    s4.toQueryStructRsp(by2);
    delete[] by2;

    cout << s4.indexes.size() << endl;
    cout << s4.dists.size() << endl;
    cout << s4.parameters.size() << endl;

    cout << s4.indexes[0] << " " << s4.indexes[1] << endl;
    cout << s4.dists[0] << " " << s4.dists[1] << endl;
    cout << s4.parameters[0] << " " << s4.parameters[1] << endl;


    return 0;
}

int srMaster(int argc, char *argv[]){
    string paramFile(argv[1]);
    map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

    map<string,string> params;
    params["bufferSize"] = "66666";
    params["port"] = "12345";

    params["bucketOffset"] = "0";
    params["bucketCount"] = "512";

    string name = "name";
    SRProcessor srp(name,params);
    srp.load("tmp");

    name = "name2";
    params["bucketOffset"] = "512";
    params["port"] = "12346";
    SRProcessor srp2(name,params);
    srp2.load("tmp");

    arma::fmat dataToIndex;
    dataToIndex.load(parameters["data"]);

    SRMaster srm(name,parameters);
    arma::fmat query = dataToIndex.col(0);
    std::pair<vector<unsigned long>,vector<float> > r = srm.knnSearchIdLong(query,10,1);

    cout << "********** RESULTS **********" << endl;
    for(int i = 0; i < r.first.size(); i++){
        cout << r.first[i] << " " << r.second[i] << endl;
    }
    cout << "*****************************" << endl;


    return 0;
}


int dataPreProcessor(int argc, char *argv[]){
    string paramFile(argv[1]);
    map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

	FeatureExtractor* sr = (FeatureExtractor*)analysers[0];

    arma::fmat dataToIndex;
    dataToIndex.load(parameters["data"]);

    uint divisions = std::stoi(parameters["divisions"]);

    std::vector<std::vector<Coefficient>> indexData(1024);

    uint sizeOfCoeff = sizeof(unsigned long)*2 + sizeof(float);
    uint numBuckets = 1024;

    uint totalSize = sizeof(uint)+sizeof(uint)*numBuckets;
    for(long i = 0; i < dataToIndex.n_cols; i++){
        arma::fmat features = dataToIndex.col(i);
        arma::fmat sparseRep;

        sr->extractFeatures(features,sparseRep);
        arma::uvec ind = find(sparseRep > 0);
        for(uint j = 0; j < ind.n_rows; j++){
            totalSize+=sizeOfCoeff;
            uint bucket = ind[j];
            uint machine = bucket/divisions;
            indexData[bucket].push_back(Coefficient(i,i,sparseRep[bucket]));
        }
    }

    uint curr = 0;
    char* dataToSave = new char[totalSize];
    memcpy(&dataToSave[curr],&numBuckets,sizeof(uint));
    curr += sizeof(uint);

    for(uint i = 0; i < indexData.size(); i++){
        uint bSize = indexData[i].size();
        memcpy(&dataToSave[curr],&bSize,sizeof(uint));
        curr += sizeof(uint);

        for(uint j = 0; j < indexData[i].size(); j++){
            Coefficient c = indexData[i][j];
            //cout << c.vector_pos << endl;
            memcpy(&dataToSave[curr],&c.vector_pos,sizeof(unsigned long));
            curr += sizeof(unsigned long);

            memcpy(&dataToSave[curr],&c.original_id,sizeof(unsigned long));
            curr += sizeof(unsigned long);

            memcpy(&dataToSave[curr],&c.value,sizeof(float));
            curr += sizeof(float);

        }
    }
    std::ofstream outfile ("tmp_coeffs.bin",std::ofstream::binary);
    outfile.write (&dataToSave[0],curr);
    outfile.close();

    dataToIndex.save("tmp_features.bin");

    curr = 0;
    uint nBuckets = *reinterpret_cast<uint*>(&dataToSave[curr]);
    curr += sizeof(uint);

    std::vector<std::vector<Coefficient>> indexData2(nBuckets);


    for(long i = 0; i < nBuckets; i++){
        uint co = *reinterpret_cast<uint*>(&dataToSave[curr]);
        curr += sizeof(uint);

        for(uint j = 0; j < co; j++){
            unsigned long vector_pos = *reinterpret_cast<unsigned long*>(&dataToSave[curr]);
            curr += sizeof(unsigned long);

            unsigned long original_id = *reinterpret_cast<unsigned long*>(&dataToSave[curr]);
            curr += sizeof(unsigned long);

            unsigned long value = *reinterpret_cast<float*>(&dataToSave[curr]);
            curr += sizeof(float);

            indexData2[i].push_back(Coefficient(vector_pos,original_id,value));
        }
    }
    /*
    cout << indexData2.size() << endl;
    cout << indexData.size() << endl;
    for(uint i = 0; i < nBuckets; i++){
        cout << indexData[i].size() << " ";
        cout << indexData2[i].size() << " ";

        if(indexData[i].size() > 0){
            cout << " " << indexData[i][0].original_id << " " << indexData2[i][0].original_id;
        }

        cout << endl;
    }
    */

    return 0;
}

int srProcessor(int argc, char *argv[]){
    string paramFile(argv[1]);
    map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

    return 0;
}


int main(int argc, char *argv[]){

    if(StringTools::endsWith(string(argv[1]),"testDistIndexer"))
        testDistIndexer(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"srMaster"))
        srMaster(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"srProcessor"))
        srProcessor(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"dataPreProcessor"))
        dataPreProcessor(argc-1,&argv[1]);


    return 0;
}
