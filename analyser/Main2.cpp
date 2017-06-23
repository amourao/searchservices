#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <memory>
#include <time.h>
//#include <math>

#include "nVector/ANdOMPExtractor.h"
#include "nVector/ANdOMPTrainer.h"

#include "../indexer/SRIndexer.h"


#include "sources/ImageSource.h"
#include "sources/CamSource.h"
#include "sources/SingleImageSource.h"
#include "sources/VideoSource.h"
#include "sources/TextFileSource.h"
#include "sources/TextFileSourceV2.h"
#include "nVector/GaborExtractor.h"
#include "nVector/LBPExtractor.h"
#include "nVector/FeatureExtractor.h"

#include "nVector/NullExtractor.h"
#include "nKeypoint/SURFExtractor.h"
#include "nVector/HistogramExtractor.h"
#include "nVector/SegmentedHistogramExtractor.h"
#include "nVector/LireExtractor.h"

#include "nVector/GISTExtractor.h"


#include "nVector/LLCExtractor.h"



#include "nRoi/FaceDetection.h"

#include "nTag/kNNClassifier.h"
#include "nTag/SVMClassifier.h"
#include "nTag/VWBasicClassifier.h"

#include "tools/TrainTestFeaturesTools.h"
#include "tools/MIRFlickrImporter.h"
#include "tools/tinyImageImporter.h"
#include "tools/oneBillionImporter.h"
#include "tools/FrameFilter.h"

#include "FactoryAnalyser.h"

#include "../indexer/FactoryIndexer.h"
#include "../indexer/IIndexer.h"
#include "../indexer/FlannkNNIndexer.h"
#include "../indexer/MSIDXIndexer.h"
#include "../indexer/DistributedIndexWrapperServer.h"
#include "../indexer/DistributedIndexWrapperClientDist.h"



#include "../commons/StringTools.h"

#include "../dataModel/DatabaseConnection.h"

#include "../commons/LoadConfig.h"

#include "../rest/Endpoints/analyser/ExtractFeatures.h"
#include "../rest/RestServer.h"


#include <ksvd/clustering.h>
#include <utils/utils.h>


using namespace std;



void testBuckets(ANdOMPExtractor fe, arma::fmat T){

    arma::fmat Dt = fe.D.t();
    int max = std::numeric_limits<int>::min();
    int min = std::numeric_limits<int>::max();
    int total = 0;

    arma::uvec nonZeroCount = zeros<uvec>(fe.getFeatureVectorSize());
    arma::fvec nonZeroSum = zeros<fvec>(fe.getFeatureVectorSize());
    double totalRecError = 0;

    for(uint i = 0; i < T.n_cols; i++){
        arma::fmat src = T.col(i);
        arma::fmat dst;
        fe.extractFeatures(src,dst);

        int count = 0;
        for(uint j = 0; j < dst.n_cols; j++){
            if(dst(0,j) > 0){
                count++;
                nonZeroCount.at(j)++;
                nonZeroSum.at(j)+= dst(0,j);
            }
        }
        if(count < min)
            min = count;
        if(count > max)
            max = count;

        total+= count;

        totalRecError+= arma::accu(arma::abs(src-(dst*Dt).t()));


    }


    for(uint j = 0; j < nonZeroCount.n_rows; j++){
            cout << nonZeroCount(j) << " \t";
    }
    cout << endl;

    for(uint j = 0; j < nonZeroSum.n_rows; j++){
            cout << nonZeroSum(j) << " \t";
    }
    cout << endl;

    cout << min << endl << max << endl << total/(int)nonZeroCount.n_rows << endl << totalRecError/T.n_cols << endl;

}


int testANdMPWithBias(int argc, char *argv[]){

    string paramFile(argv[1]);
    string bias(argv[2]);
    double expon, regFactor, weight;

    bool withBias = bias == "-bias";

    if (withBias){
        expon = stod(string(argv[3]));
        regFactor = stod(string(argv[4]));
        weight = stod(string(argv[5]));
    }



	map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);


    int dimensionality = std::stoi(parameters["dimensionality"]);
    string type = "a";
    map<string,string> params;
    params = parameters;

	params["dimensions"] = std::to_string(dimensionality);
	params["dimensionality"] = std::to_string(dimensionality);
    params["iters"] = "25";

	//params["eps"] = "1e-7";
	//params["max_iters"] = "10";
	//params["eps_ksvd"] = "1e-7";
	//params["max_iters_ksvd"] = "10";

	params["normalize"] = "cols";
	params["dictPath"] = parameters["dictPath"];

    arma::fmat T,VI,VQ;
    T.load(parameters["T"]);
    VI.load(parameters["VI"]);
    VQ.load(parameters["VQ"]);

    arma::fmat means = arma::mean(T);
    arma::fmat stddevs = arma::stddev(T);

    for(uint i = 0; i < T.n_cols; i++){
        if(stddevs.at(0,i) == 0)
            T.col(i) = (T.col(i) - means.at(0,i));
        else
            T.col(i) = (T.col(i) - means.at(0,i))/stddevs.at(0,i);
    }

    params["dictSize"] = std::to_string(T.n_cols);


    ANdOMPExtractor andEx(type,params);

    arma::fmat dictionary_seed = andEx.D;
    ANdOMPTrainer andTr(andEx,stoi(params["iters"]),stod(params["eps_ksvd"]),dimensionality, expon, regFactor, weight,withBias,false);
    //ANdOMPExtractor _fe, int _n_iters, double _eps, uint _dimensions, double _expon, double _regFactor, double _weight, bool _withBias
    andTr.train(dictionary_seed,T,VI,VQ);
    andTr.D.save("workingDir/andTr_D_" + bias + "_" + std::to_string(expon) + "_" + std::to_string(regFactor) + "_" + std::to_string(weight) + ".bin");

    andEx.changeDictionary(andTr.D);
    testBuckets(andEx,T);


    /*
    arma::fmat doo = sr.dictionary_seed;
    arma::fmat doo2 = andTr.D;

    for(int x = 0; x < doo.n_cols; x++)
        for(int y = 0; y < doo.n_rows; y++)
            cout << doo.at(x,y) << "\t";
    cout << endl;

    doo = sr.dictionary;

    for(int x = 0; x < doo.n_cols; x++)
        for(int y = 0; y < doo.n_rows; y++)
            cout << doo.at(x,y) << "\t";
    cout << endl;
    for(int x = 0; x < doo2.n_cols; x++)
        for(int y = 0; y < doo2.n_rows; y++)
            cout << doo2.at(x,y) << "\t";
    cout << endl;
    for(int x = 0; x < doo2.n_cols; x++)
        for(int y = 0; y < doo2.n_rows; y++)
            cout << doo2.at(x,y)-doo.at(x,y) << "\t";
    cout << endl;
    */


    std::cout << "trained KSVD ok" << endl;


    return 0;
}



int validateANdMPWithBias(int argc, char *argv[]){

    string paramFile(argv[1]);
    string dictio(argv[2]);

	map<string,string> parameters;
	vector<IIndexer*> indexers;
	vector<IAnalyser*> analysers;
	vector<IClassifier*> classifiers;
	vector<IEndpoint*> endpoints;

	LoadConfig::load(paramFile,parameters,indexers,analysers,classifiers,endpoints);


    int dimensionality = std::stoi(parameters["dimensionality"]);
    string type = "a";
    map<string,string> params;
    params = parameters;

	params["dimensions"] = std::to_string(dimensionality);
	params["dimensionality"] = std::to_string(dimensionality);
    params["iters"] = "25";

	//params["eps"] = "1e-7";
	//params["max_iters"] = "10";
	//params["eps_ksvd"] = "1e-7";
	//params["max_iters_ksvd"] = "10";

	params["normalize"] = "cols";
	params["dictPath"] = parameters["dictPath"];

    arma::fmat VI,VQ, D;
    VI.load(parameters["VI"]);
    VQ.load(parameters["VQ"]);
    D.load(dictio);

    arma::fmat means = arma::mean(VQ);
    arma::fmat stddevs = arma::stddev(VQ);

    for(uint i = 0; i < VQ.n_cols; i++){
        if(stddevs.at(0,i) == 0)
            VQ.col(i) = (VQ.col(i) - means.at(0,i));
        else
            VQ.col(i) = (VQ.col(i) - means.at(0,i))/stddevs.at(0,i);
    }

    params["dictSize"] = std::to_string(VQ.n_cols);


    ANdOMPExtractor andEx(type,params);

    andEx.changeDictionary(D);
    testBuckets(andEx,VQ);


    /*
    arma::fmat doo = sr.dictionary_seed;
    arma::fmat doo2 = andTr.D;

    for(int x = 0; x < doo.n_cols; x++)
        for(int y = 0; y < doo.n_rows; y++)
            cout << doo.at(x,y) << "\t";
    cout << endl;

    doo = sr.dictionary;

    for(int x = 0; x < doo.n_cols; x++)
        for(int y = 0; y < doo.n_rows; y++)
            cout << doo.at(x,y) << "\t";
    cout << endl;
    for(int x = 0; x < doo2.n_cols; x++)
        for(int y = 0; y < doo2.n_rows; y++)
            cout << doo2.at(x,y) << "\t";
    cout << endl;
    for(int x = 0; x < doo2.n_cols; x++)
        for(int y = 0; y < doo2.n_rows; y++)
            cout << doo2.at(x,y)-doo.at(x,y) << "\t";
    cout << endl;
    */


    std::cout << "Validating dictionary ok" << endl;


    return 0;
}


int main(int argc, char *argv[])
{
    srand (time(NULL));
	//testLoadSaveIClassifier(argc, argv);
	//testLoadSaveIIndexer(argc, argv);
	//faceDetectionParameterChallenge(argc, argv);
    //testAllClassifiersBin(argc, argv);
    //createMedCatClassifier(argc, argv);
    //testDatabaseConnection(argc, argv);
    //classifyAllImagesCondor(argc, argv);

    //extractAllFeaturesCKv2(argc, argv);
    //merger(argc, argv);
    //extractAllFeaturesCK(argc, argv);
	//testMSIDXIndexer(argc, argv);

    //classifyAllBlipImagesCondor(argc, argv);
    //classifyAllBlipImagesCondor(argc, argv);

    //FrameFilter::maine(argc, argv);
    //classifySapoAllVideos(argc, argv);
    //extractREST(argc, argv);
	//createBlipKnnVWDict(argc, argv);

    //extractAndSaveToBerkeleyDB(argc, argv);
	//readBerkeleyDB(argc, argv);
	for(int i = 0; i < argc; i++)
        cout << argv[i] << endl;

    if(StringTools::endsWith(string(argv[1]),"testANdMPWithBias"))
        testANdMPWithBias(argc-1,&argv[1]);
    else if(StringTools::endsWith(string(argv[1]),"validateANdMPWithBias"))
        validateANdMPWithBias(argc-1,&argv[1]);


    return 0;
}

