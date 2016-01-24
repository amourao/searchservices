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

//#define ELPP_THREAD_SAFE

//#include <easylogging++.h>

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

//INITIALIZE_EASYLOGGINGPP

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

    vector<SRProcessor<float>*> ser;

    string name = "srMaster";
    arma::fmat dataToIndex;
    dataToIndex.load(parameters["dataQ"]);

    SRMaster srm(name,parameters);

    uint nQueries = std::stoi(parameters["nQueries"]);

    //#pragma omp parallel for schedule(dynamic)
    for(uint i = 0; i < nQueries; i++){
        arma::fmat query = dataToIndex.col(i);
        std::pair<vector<uindex>,vector<float> > r = srm.knnSearchIdLong(query,std::stoi(parameters["n"]),std::stod(parameters["limit"]));

        cout << "********** RESULTS **********" << endl;
        cout << "************* " << i << " ************" << endl;
        for(uint j = 0; j < r.first.size(); j++){
            cout << r.first[j] << "\t" << r.second[j] << endl;
        }
        cout << "*****************************" << endl;
    }

    cout << endl;
    cout << srm.totalQueryTime << endl;
    cout << srm.totalNQueries << endl << endl;
    cout << srm.totalQueryTime/srm.totalNQueries/1000.0 << endl << endl;
    cout << srm.totalAllServerTime/srm.totalNQueries/1000.0 << endl << endl;

    cout << srm.totalSRTime/srm.totalNQueries/1000.0 << endl;
    cout << srm.totalPreMarshallingTime/srm.totalNQueries/1000.0 << endl;

    cout << srm.totalCommunicationTime/srm.totalNQueries/1000.0 << endl;

    cout << "Q\t" << srm.totalCommunicationSendTime/srm.totalNQueries/1000.0 << endl;
    cout << "Q\t" << srm.totalCommunicationReceiveTime/srm.totalNQueries/1000.0 << endl;

    cout << "R\t\t" << srm.totalCommunicationSendTime/srm.totalRequests/1000.0 << endl;
    cout << "R\t\t" << srm.totalCommunicationReceiveTime/srm.totalRequests/1000.0 << endl;

    cout << srm.totalSortTime/srm.totalNQueries/1000.0 << endl << endl;

    cout << "Missed packages: " << srm.missedPackages << endl << endl;
    cout << "Total requests: " << srm.totalRequests << endl << endl;


    cout << "Processor stats: " << endl;
    srm.printProcessorsStatistics();

    return 0;

}

int srTest(int argc, char *argv[]){
    string paramFile(argv[1]);
    map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);




    uint nServers = std::stoi(parameters["divisions"]);
    uint nBuckets = std::stoi(parameters["nBuckets"]);
    uint startPort = std::stoi(parameters["port"])+1;
    uint nBucketsPerServer = nBuckets / nServers;

    uint accum = std::stoi(parameters["bufferOffset"]);

    map<string,string> params;

    vector<SRProcessor<float>*> ser;

    parameters["servers"] = "";
    params["bufferSize"] = parameters["bufferSize"];
    for (uint i = 0; i < nServers; i++){

        params["port"] = std::to_string(startPort++);
        parameters["servers"] += "localhost:" + params["port"];

        params["bucketOffset"] = std::to_string(accum);


        if (i == nServers-1){
            params["bucketCount"] = std::to_string(nBuckets-accum);
            accum=nBuckets;
        } else {
            params["bucketCount"] = std::to_string(nBucketsPerServer);
            accum+=nBucketsPerServer;

            parameters["servers"] +=  ";";
        }
        string name = "srProcessor_" + std::to_string(i);
        SRProcessor<float>* srp = new SRProcessor<float>(name,params);
        srp->load("tmp");
        ser.push_back(srp);
    }
    string name = "srMaster";
    arma::fmat dataToIndex;
    dataToIndex.load(parameters["dataQ"]);

    SRMaster srm(name,parameters);

    uint nQueries = std::stoi(parameters["nQueries"]);

    //#pragma omp parallel for schedule(dynamic)
    for(uint i = 0; i < nQueries; i++){
        arma::fmat query = dataToIndex.col(i);
        std::pair<vector<uindex>,vector<float> > r = srm.knnSearchIdLong(query,std::stoi(parameters["n"]),std::stod(parameters["limit"]));

        cout << "********** RESULTS **********" << endl;
        cout << "************* " << i << " ************" << endl;
        for(uint j = 0; j < r.first.size(); j++){
            cout << r.first[j] << "\t" << r.second[j];
        }
        cout << "*****************************" << endl;
    }


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

    std::vector<std::vector<Coefficient>> indexData(1024);

    uint sizeOfCoeff = sizeof(uindex)*1 + sizeof(float);
    uint numBuckets = 1024;
    //uint divisions = std::stoi(parameters["divisions"]);

    uint totalSize = sizeof(uint)+sizeof(uint)*numBuckets;

    std::vector<uint> lidToGid(dataToIndex.n_cols);

    for(uint i = 0; i < dataToIndex.n_cols; i++){
        lidToGid[i]=i;
        arma::fmat features = dataToIndex.col(i);
        arma::fmat sparseRep;

        sr->extractFeatures(features,sparseRep);
        arma::uvec ind = find(sparseRep > 0);
        for(uint j = 0; j < ind.n_rows; j++){
            totalSize+=sizeOfCoeff;
            uint bucket = ind[j];
            //uint machine = bucket/divisions;
            indexData[bucket].push_back(Coefficient(i,sparseRep[bucket]));
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
            memcpy(&dataToSave[curr],&c.vector_pos,sizeof(uindex));
            curr += sizeof(uindex);

            memcpy(&dataToSave[curr],&c.value,sizeof(float));
            curr += sizeof(float);

        }
    }

    string dst = parameters["dst"];
    std::ofstream outfile (dst + "_coeffs.bin",std::ofstream::binary);
    outfile.write (&dataToSave[0],curr);
    outfile.close();

    dataToIndex.save(dst + "_features.bin");

    std::ofstream outfile2 (dst + "_lidtogid.bin",std::ofstream::binary);
    outfile2.write ((char*)&lidToGid[0],sizeof(uint)*lidToGid.size());
    outfile2.close();

    curr = 0;
    uint nBuckets = *reinterpret_cast<uint*>(&dataToSave[curr]);
    curr += sizeof(uint);

    std::vector<std::vector<Coefficient>> indexData2(nBuckets);


    for(long i = 0; i < nBuckets; i++){
        uint co = *reinterpret_cast<uint*>(&dataToSave[curr]);
        curr += sizeof(uint);

        for(uint j = 0; j < co; j++){
            uindex vector_pos = *reinterpret_cast<uindex*>(&dataToSave[curr]);
            curr += sizeof(uindex);

            uindex value = *reinterpret_cast<float*>(&dataToSave[curr]);
            curr += sizeof(float);

            indexData2[i].push_back(Coefficient(vector_pos,value));
        }
    }


    map<string,string> params;
    params["bufferSize"] = std::to_string(66666);

    params["port"] = std::to_string(54232);
    params["bucketOffset"] = std::to_string(0);
    params["bucketCount"] = std::to_string(512);

    string name = "srProcessor_";
    SRProcessor<float>* srp = new SRProcessor<float>(name,params);
    srp->load(dst);

    cout << indexData2.size() << endl;
    cout << indexData.size() << endl;
    cout << srp->indexData.size() << endl;
    cout << srp->data.n_cols << endl;
    for(uint i = 0; i < 512; i++){
        cout << indexData[i].size() << " ";
        cout << indexData2[i].size() << " ";
        cout << srp->indexData[i].size() << " ";

        if(indexData[i].size() > 0){
            cout << " " << indexData[i][0].vector_pos << " " << indexData2[i][0].vector_pos << " " << srp->lidTogid[srp->indexData[i][0].vector_pos]  << " " <<  srp->indexData[i][0].vector_pos;
        }

        cout << endl;
    }

    params["port"] = std::to_string(54233);
    params["bucketOffset"] = std::to_string(512);
    params["bucketCount"] = std::to_string(512);

    SRProcessor<float>* srp2 = new SRProcessor<float>(name,params);
    srp2->load(dst);

    cout << indexData2.size() << endl;
    cout << indexData.size() << endl;
    cout << srp2->indexData.size() << endl;
    cout << srp2->data.n_cols << endl;
    for(uint i = 512; i < nBuckets; i++){
        cout << indexData[i].size() << " ";
        cout << indexData2[i].size() << " ";
        cout << srp2->indexData[i-512].size() << " ";

        if(indexData[i].size() > 0){
            cout << " " << indexData[i][0].vector_pos << " " << indexData2[i][0].vector_pos << " " <<  srp2->lidTogid[srp2->indexData[i-512][0].vector_pos]  << " " << srp2->indexData[i-512][0].vector_pos;
        }

        cout << endl;
    }

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

    uint divisions = std::stoi(parameters["divisions"]);
    uint nBuckets = std::stoi(parameters["bucketCount"]);
    uint startPort = std::stoi(parameters["port"]);
    uint nBucketsPerServer = nBuckets/divisions;

    uint accum = std::stoi(parameters["bucketOffset"]);

    string path = parameters["path"];

    map<string,string> params;

    vector<SRProcessor<float>*> ser;

    params["bufferSize"] = parameters["bufferSize"];
    for (uint i = 0; i < divisions; i++){

        params["port"] = std::to_string(startPort++);
        params["bucketOffset"] = std::to_string(accum);
        params["bucketCount"] = std::to_string(nBucketsPerServer);
        accum+=nBucketsPerServer;

        string name = "srProcessor_" + std::to_string(i);
        SRProcessor<float>* srp = new SRProcessor<float>(name,params);
        srp->load(path);
        ser.push_back(srp);
    }

    //for(;;){}
    //pthread_exit(NULL);
    for (uint i = 0; i < ser.size(); i++){
        ser.at(i)->_thread.join();
    }

    return 0;
}



int srProcessorBillion(int argc, char *argv[]){
    string paramFile(argv[1]);
    map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);

    uint divisions = std::stoi(parameters["divisions"]);
    uint nBuckets = std::stoi(parameters["bucketCount"]);
    uint startPort = std::stoi(parameters["port"]);
    uint nBucketsPerServer = nBuckets/divisions;

    uint accum = std::stoi(parameters["bucketOffset"]);


    string path = parameters["pathData"];
    string coeffs = parameters["pathCoeffs"];

    map<string,string> params;

    vector<SRProcessor<float>*> ser;

    params["bufferSize"] = parameters["bufferSize"];
    for (uint i = 0; i < divisions; i++){

        params["port"] = std::to_string(startPort++);
        params["bucketOffset"] = std::to_string(accum);
        params["bucketCount"] = std::to_string(nBucketsPerServer);
        accum+=nBucketsPerServer;

        string name = "srProcessor_" + std::to_string(i);
        SRProcessor<float>* srp = new SRProcessor<float>(name,params);
        srp->loadBilion(path,coeffs);
        ser.push_back(srp);
    }

    //for(;;){}
    //pthread_exit(NULL);
    for (uint i = 0; i < ser.size(); i++){
        ser.at(i)->_thread.join();
    }

    return 0;
}



int matSizeTest(int argc, char *argv[]){

    arma::Mat<char> teste(2,2,arma::fill::zeros);

    teste(0,0) = 1;
    teste(0,1) = 2;
    teste(1,0) = 3;
    teste(1,1) = 4;

    arma::Mat<int> teste2(2,2,arma::fill::zeros);

    teste2(0,0) = 1;
    teste2(0,1) = 2;
    teste2(1,0) = 3;
    teste2(1,1) = 4;

    arma::Mat<float> teste3(2,2,arma::fill::zeros);

    teste3(0,0) = 1;
    teste3(0,1) = 2;
    teste3(1,0) = 3;
    teste3(1,1) = 4;

    tp totalSortTimeStart = NOW();
    /*
    cout << "\t" << myNorm(teste) << endl;
    cout << ELAPSED(totalSortTimeStart) << endl;
    totalSortTimeStart = NOW();
    cout << "\t" << myNorm(teste2) << endl;
    cout << ELAPSED(totalSortTimeStart) << endl;
    totalSortTimeStart = NOW();
    cout << "\t" << myNorm(teste3) << endl;
    cout << ELAPSED(totalSortTimeStart) << endl;
    totalSortTimeStart = NOW();
    cout << "\t" << myNorm2(teste3) << endl;
    cout << ELAPSED(totalSortTimeStart) << endl << endl;
    */
    oneBillionImporterB ob;

    arma::Mat<uchar> featuresB;
    arma::Mat<uchar> featuresC;

    arma::Mat<uchar> featuresE;
    arma::Mat<uchar> featuresF;

    arma::Mat<uchar> featuresH;
    arma::Mat<uchar> featuresI;




    totalSortTimeStart = NOW();
    ob.readBin("/localstore/1-billion-vectors/queries.bvecs",10000,featuresB,0);
    cout << ELAPSED(totalSortTimeStart) << endl;

    totalSortTimeStart = NOW();
    ob.readBinSlow("/localstore/1-billion-vectors/queries.bvecs",10000,featuresC,0);
    cout << ELAPSED(totalSortTimeStart) << endl;

    totalSortTimeStart = NOW();
    ob.readBin("/localstore/1-billion-vectors/queries.bvecs",10000,featuresE,0);
    cout << ELAPSED(totalSortTimeStart) << endl;

    totalSortTimeStart = NOW();
    ob.readBinSlow("/localstore/1-billion-vectors/queries.bvecs",10000,featuresF,0);
    cout << ELAPSED(totalSortTimeStart) << endl;

    cout << (int)arma::accu(featuresF-featuresE) << endl;




    vector<uint> buckets;


    buckets.push_back(4);
    buckets.push_back(3);
    buckets.push_back(2);
    buckets.push_back(1);
    buckets.push_back(0);

    totalSortTimeStart = NOW();
    ob.readBin("/localstore/1-billion-vectors/queries.bvecs",featuresH,buckets);
    cout << ELAPSED(totalSortTimeStart) << endl;

    totalSortTimeStart = NOW();
    ob.readBinSlow("/localstore/1-billion-vectors/queries.bvecs",5,featuresI,0);
    cout << ELAPSED(totalSortTimeStart) << endl;

    cout << featuresH << endl;
    cout << featuresI << endl;

    //cout << featuresB.n_rows << " " << featuresB.n_cols << endl;
    //cout << featuresB << endl;
    //featuresB.save("tmpData.bin");

    //arma::fmat featuresBF;
    //ob.readBin("/localstore/1-billion-vectors/queries.bvecs",5,featuresBF,0);
    //cout << featuresBF.n_rows << " " << featuresBF.n_cols << endl;
    //cout << featuresBF << endl;

    return 0;
}



int armaToBin(int argc, char *argv[]){

    string filename(argv[1]);
    string filenameOut(argv[2]);

    arma::field<arma::vec>  F;
    F.load(filename);

    /*
    arma::field<arma::vec> F1(10,2);

    for (arma::uword j = 0; j < 10; ++j)
    {
        F1(j,0) = arma::vec(j);
        F1(j,1) = arma::vec(j);
        for (arma::uword i = 0; i < j; ++i)
        {
            F1(j,0)(i) = (uint)10;
            F1(j,1)(i) = (float)0.5f;
        }
    }
    F1.save(filename);
    */
    cout << F.n_rows << " " << F.n_cols << " " << F.size() << " " << endl;

    FILE* pFile;
    pFile = fopen(filenameOut.c_str(), "wb");

    uint nBuckets = F.n_rows;
    fwrite((char*)&nBuckets, 1, 1*sizeof(uint), pFile);

    for (arma::uword j = 0; j < nBuckets; ++j)
    {
        uint size = F(j,0).size();
        fwrite((char*)&size, 1, 1*sizeof(uint), pFile);

        //cout << size << endl;
        for (arma::uword i = 0; i < F(j,0).size(); ++i)
        {
            uint vecpos = F(j,0)[i];
            float coeff = F(j,1)[i];
            fwrite((char*)&vecpos, 1, 1*sizeof(uint), pFile);
            fwrite((char*)&coeff, 1, 1*sizeof(float), pFile);
            //cout << vecpos << " " << coeff << endl;
        }
    }
    fclose(pFile);

    return 0;
}

int oneBillionToArmaMat(int argc, char *argv[]){

    oneBillionImporterB ob;
    arma::Mat<uchar> featuresB;

    tp totalSortTimeStart = NOW();
    ob.readBin("/localstore/1-billion-vectors/queries.bvecs",10000,featuresB,0);
    cout << ELAPSED(totalSortTimeStart) << endl;
    return 0;
}

int main(int argc, char *argv[]){
    //el::Helpers::setCrashHandler(myCrashHandler);
    //el::Loggers::addFlag( el::LoggingFlag::DisableApplicationAbortOnFatalLog );
    //el::Loggers::addFlag( el::LoggingFlag::ColoredTerminalOutput );

    /*
    cout << sizeof(char) << endl;
    cout << sizeof(short) << endl;
    cout << sizeof(int) << endl;
    cout << sizeof(uint) << endl;
    cout << sizeof(long) << endl;
    cout << sizeof(unsigned long) << endl;
    cout << sizeof(float) << endl;
    cout << sizeof(double) << endl;
    */

    if(StringTools::endsWith(string(argv[1]),"testDistIndexer"))
        testDistIndexer(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"srMaster"))
        srMaster(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"srProcessor"))
        srProcessor(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"srProcessorBillion"))
        srProcessorBillion(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"srTest"))
        srTest(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"dataPreProcessor"))
        dataPreProcessor(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"matSizeTest"))
        matSizeTest(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"armaToBin"))
        armaToBin(argc-1,&argv[1]);




    return 0;
}
