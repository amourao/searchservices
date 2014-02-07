#include <iostream>
#include <string>
#include <sstream>
#include <set>
#include <cstdlib>
#include <opencv2/features2d/features2d.hpp>
#include <time.h>
//#include <math>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/autolink.h>

#include "nTag/SRClassifier.h"

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

#include "nRoi/FaceDetection.h"

#include "nTag/kNNClassifier.h"
#include "nTag/SVMClassifier.h"
#include "nTag/VWBasicClassifier.h"

#include "tools/TrainTestFeaturesTools.h"
#include "tools/MIRFlickrImporter.h"
#include "tools/tinyImageImporter.h"

#include "FactoryAnalyser.h"

#include "../indexer/FactoryIndexer.h"
#include "../indexer/IIndexer.h"
#include "../indexer/FlannkNNIndexer.h"
#include "../indexer/MSIDXIndexer.h"
#include "../indexer/LSHIndexer.h"

#include "../commons/StringTools.h"
#include "../commons/Timing.h"




using namespace std;


struct tagError {
     double error;
     string tag;
 };

class  CompareTags{
     public:
     bool operator()(tagError& t1, tagError& t2){
        return t1.error > t2.error;
     }
 };

void testLoadSaveIIndexer(int argc, char *argv[]){
	string file(argv[1]);

	Mat features;
	Mat labels;

	MatrixTools::readBin(file, features, labels);
	IIndexer* vw = new FlannkNNIndexer();

	vw->index(features);
	vw->save("medicalImage_CEDD_kNN");
	Mat q = features.row(0);

	vector<std::pair<float,float> > r = vw->knnSearchId(q,10);
	for(uint i = 0; i < r.size(); i++){
		cout << r.at(i).first << "\t" << r.at(i).second << endl;
	}
	cout  << endl;
	delete vw;
	vw = new FlannkNNIndexer();

	vw->load("medicalImage_CEDD_kNN");

	r = vw->knnSearchId(q,10);

	for(uint i = 0; i < r.size(); i++){
		cout << r.at(i).first << "\t" << r.at(i).second << endl;
	}
}

void generatePermutations(Json::Value root, vector<map<string,string> >& result)
{
    vector<vector<string> > lists;

    vector<string> mapIntName;


    vector<map<string,string> > combinations;
    vector<map<string,string> > newCombinations;

    int i = 0;
    for( Json::ValueIterator itr = root.begin(); itr != root.end(); itr++ ){
        vector<string> innerList;
        Json::Value innerArray = root[itr.key().asString()];
        mapIntName.push_back(itr.key().asString());

        if (innerArray.isArray()){
            for ( int j = 0; j < innerArray.size(); j++ ){
                innerList.push_back(innerArray[j].asString());
            }
        } else {
            innerList.push_back(innerArray.asString());
        }
        lists.push_back(innerList);
        i++;
    }



    for(int i = 0; i < lists.at(0).size(); i++){
        map<string,string> inner;
        inner[mapIntName.at(0)] = lists.at(0).at(i);
        combinations.push_back(inner);
    }

    for(int i = 1; i < lists.size(); i++){
        vector<string> next = lists.at(i);
        newCombinations = vector<map<string,string> >();
        for (vector<map<string,string> >::iterator it=combinations.begin(); it!=combinations.end(); ++it){ // *it
            for (vector<string>::iterator it2=next.begin(); it2!=next.end(); ++it2){ // *it2
                (*it)[mapIntName.at(i)] = (*it2);
                newCombinations.push_back((*it));

            }
        }
        combinations = newCombinations;
    }


    for (int i = 0; i < combinations.size(); i++){
        for (map<string,string>::iterator it=combinations.at(i).begin(); it!=combinations.at(i).end(); ++it){
            cout << it->first << " " << it->second << endl;
        }
    }
    result = combinations;
}

vector<IIndexer*> testRegisteringIndeces(char *argv){


    vector<IIndexer*> indexers;

    Json::Value root;   // will contains the root value after parsing.
    Json::Reader reader;

    std::fstream file (argv, std::fstream::in | std::fstream::out);

    bool parsingSuccessful = reader.parse( file, root );

    const Json::Value plugins = root["endpoints"]["indexer"];

    for ( int i = 0; i < plugins.size(); i++ ){
        const Json::Value p = plugins[i];

        string newName = p["newName"].asString();
        string originalName = p["originalName"].asString();

        Json::Value paramsJSON = p["params"];

        map<string,string> params;
        vector<map<string,string> > allParams;

        generatePermutations(paramsJSON,allParams);


        for( int j = 0; j < allParams.size(); j++){
            map<string,string> params = allParams.at(j);
            stringstream ss;
            ss << newName << "_" << j;
            string newNameId = ss.str();
            IIndexer* originalIndex = (IIndexer*)FactoryIndexer::getInstance()->createType(originalName);
            FactoryIndexer::getInstance()->registerType(newNameId,originalIndex,params);
            IIndexer* readyIndex = (IIndexer*)FactoryIndexer::getInstance()->createType(newNameId);

            indexers.push_back(readyIndex);
        }
    }

    return indexers;
}




void testIndeces(int argc, char *argv[]){
	string file(argv[1]);

	int n = atoi(argv[2]);
	int nTrain = atoi(argv[2]);
	int nVal = atoi(argv[2]);
	int k = atoi(argv[3]);

	Mat features;
	Mat featuresValidation;
	Mat featuresTest;
	//Mat labels;

	tinyImageImporter::readBin(file,n,features);
	tinyImageImporter::readBin(file,n*0.1,featuresValidation,n);
	tinyImageImporter::readBin(file,n*0.1,featuresValidation,n);

	vector<IIndexer*> indexers = testRegisteringIndeces("config.json");

	timestamp_type start, end;

	cout << "Indexing" << endl;
	for(int i = 0; i < indexers.size(); i++){
		get_timestamp(&start);

		if (indexers.at(i)->getName() == "E2LSHIndexer")
            ((LSHIndexer*)indexers.at(i))->index(features,featuresValidation);
		else
            indexers.at(i)->index(features);

		get_timestamp(&end);
		cout << indexers.at(i)->getName() << " " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
	}

	cout << endl << "Querying" << endl;
	for(int i = 0; i < indexers.size(); i++){

		Mat q = features.row(0);
		get_timestamp(&start);
		vector<std::pair<float,float> > r = indexers.at(i)->knnSearchId(q,k);
		get_timestamp(&end);
		cout << indexers.at(i)->getName() << " " << timestamp_diff_in_milliseconds(start, end) << " ms" << endl;
		for(uint i = 0; i < r.size(); i++){
			cout << r.at(i).first << "\t" << r.at(i).second << endl;
		}
	}
}

int main(int argc, char *argv[]){
	testIndeces(argc, argv);
    return 0;
}
