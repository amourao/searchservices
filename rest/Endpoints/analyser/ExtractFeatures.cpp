#include "ExtractFeatures.h"

static ExtractFeatures extractFeaturesEndpointFactory;

ExtractFeatures::ExtractFeatures(string type){
  this->type = type;
}

ExtractFeatures::ExtractFeatures(){

	FactoryEndpoint::getInstance()->registerType("/analyser",this);
	FactoryEndpoint::getInstance()->registerType("/analyserSingleFile",this);

}

ExtractFeatures::~ExtractFeatures()
{}

void* ExtractFeatures::createType(string& type){
	//TODO
	std::cout << "New type requested: " << type << std::endl;

	if (type == "/analyser" || (type == "/analyserSingleFile"))
		return new ExtractFeatures(type);

	std::cerr << "Error registering type from constructor (this should never happen)" << std::endl;
	return NULL;
}


void ExtractFeatures::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  	if(method != "GET"){
		resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		resp.send();
		return ;
	}

	resp.setContentType("application/json");
	std::ostream& out = resp.send();

	std::string response("");

	if (type == "/analyser")
		response = getFeatures(queryStrings);
	else if (type == "/analyserSingleFile")
		response = getFeaturesSingle(queryStrings);

	//std::cout << response << std::endl;
	out << response;
	out.flush();


}

string ExtractFeatures::getFeaturesSingle(map<string, string > parameters){

    Json::Value root;

    try{

        FileDownloader fd;


        int filenameStatus;
        string filename = fd.getFile(parameters["url"],filenameStatus);
        string extractorName = parameters["extractor"];


        FactoryAnalyser * f = FactoryAnalyser::getInstance();

        IAnalyser* extractor = (IAnalyser*)f->createType(extractorName);


        IDataModel* data;

        if (filenameStatus < 0){
            root["message"] = "File does not exist/Error reading file: " + std::to_string(filenameStatus);
            root["status"] = "error";
        } else if (extractor == NULL){
            root["message"] = "Extractor does not exist";
            root["status"] = "error";
        } else if (extractor->getType() == IDataModel::NVECTOR){
            data = extractor->getFeatures(filename);

            vector<float>* features = (vector<float>*) data->getValue();

            Json::Value featureArray(Json::arrayValue);
            for (uint i = 0; i < features->size(); i++){
                featureArray.append(features->at(i));
                //cout << features->at(i) << " " ;
            }
            root["result"] = featureArray;
            root["type"] = "nVector";
            root["status"] = "ok";


        } else if (extractor->getType() == IDataModel::NKEYPOINT){
            data = extractor->getFeatures(filename);

            vector<pair<vector<float>,vector<float> > >* features = (vector<pair<vector<float>,vector<float> > >*) data->getValue();
            Json::Value featurePointArray(Json::arrayValue);
            for (auto &point : *features){
                vector<float> pointInfo = point.first;
                vector<float> featureVec = point.second;

                Json::Value featurePoint;
                Json::Value featureArray(Json::arrayValue);

                featurePoint["x"] = pointInfo.at(0);
                featurePoint["y"] = pointInfo.at(1);
                featurePoint["angle"] = pointInfo.at(2);
                featurePoint["size"] = pointInfo.at(3);
                featurePoint["response"] = pointInfo.at(4);
                featurePoint["octave"] = pointInfo.at(5);
                featurePoint["class_id"] = pointInfo.at(6);

                for (uint i = 0; i < featureVec.size(); i++){
                    featureArray.append(featureVec.at(i));
                    //cout << features->at(i) << " " ;
                }

                featurePoint["features"] = featureArray;

                featurePointArray.append(featurePoint);
            }
            root["result"] = featurePointArray;
            root["type"] = "nKeypoint";
            root["status"] = "ok";

        } else if (extractor->getType() == IDataModel::NROI){
            data = extractor->getFeatures(filename);

            map<string,region>* features = (map<string,region>*) data->getValue();
            Json::Value featurePointArray(Json::arrayValue);
            for(auto const &pairF : *features) {
                region r = pairF.second;

                Json::Value featurePoint;
                featurePoint["x"] = r.x;
                featurePoint["y"] = r.y;
                featurePoint["width"] = r.width;
                featurePoint["height"] = r.height;
                featurePoint["type"] = r.annotationType;
                featurePoint["name"] = pairF.first;

                featurePointArray.append(featurePoint);
            }
            root["result"] = featurePointArray;
            root["type"] = "nRoi";
            root["status"] = "ok";

        } else {
            root["message"] = "Unknown data type to output";
            root["status"] = "error";
        }
    } catch(exception& e){
        root["message"] = e.what();
        root["status"] = "error";
    } catch(...){
        root["message"] = "Unknown error";
        root["status"] = "error";
    }
    Json::FastWriter writter;
	return writter.write(root);
}

void ExtractFeatures::exportLabels(string filename, vector<map<string, int> > labels){
	for (uint i = 0; i < labels.size(); i++){
		std::map<string,int>::iterator iter;
		std::map<int,string>::iterator iter2;

		std::map<int,string> sortedMap;

		stringstream ss;

		ss << filename << "_" << i << LABELS_EXT_EXTRACT_FEATURES;

	    ofstream labelData(ss.str().c_str());

	    labelData << labels.at(i).size() << endl;

   	    for (iter = labels.at(i).begin(); iter != labels.at(i).end(); ++iter) {
	    	sortedMap[iter->second] = iter->first;
	    }


	    for (iter2 = sortedMap.begin(); iter2 != sortedMap.end(); ++iter2) {
	    	labelData << iter2->first << "," << iter2->second << endl;
	    }


		labelData.close();
	}

}

string ExtractFeatures::getFeatures(map<string, string > parameters){

	FileDownloader fd;

	string filename = parameters["input"];
	string analyserName = parameters["analyser"];
	string outputLocation = parameters["output"];

    bool filter = false;
	if(parameters.count("filter") > 0){
        filter = parameters["filter"] == "true";
	}

	FactoryAnalyser * f = FactoryAnalyser::getInstance();

	IAnalyser* analyser= (IAnalyser*)f->createType(analyserName);

	Mat src, features, labels;

	TextFileSourceV2 is(filename);

	int imageInfoFieldCount = is.getImageInfoFieldCount();
	int classFieldId = is.getClassFieldId();
	int indexFieldId = is.getIndexFieldId();

	vector<map<string,int> > idsToInternalIds (imageInfoFieldCount);

	for (int k = 0; k < is.getImageCount(); k++) {
		if (!(src = is.nextImage()).empty()) { // src contains the image, but the IAnalyser interface needs a path
			if(!filter || FrameFilter::hasEdges(src)){

                cv::Mat featuresRow;

                cv::Mat label;
                if (classFieldId == indexFieldId)
                    label = cv::Mat (1, imageInfoFieldCount+1, CV_32F);
                else
                    label = cv::Mat (1, imageInfoFieldCount, CV_32F);


                //parse the label from the info in the txt file
                string path, id1, id2;
                //stringstream liness(is.getImageInfo());

                vector<string> info = is.getCurrentImageInfoVector();

                for(int i = 0; i < imageInfoFieldCount; i++)
                    if (idsToInternalIds.at(i).count(info.at(i)) == 0)
                        idsToInternalIds.at(i)[info.at(i)] = idsToInternalIds.at(i).size()-1;

                label.at<float>(0, 0) = idsToInternalIds.at(indexFieldId)[info.at(indexFieldId)];
                label.at<float>(0, 1) = idsToInternalIds.at(classFieldId)[info.at(classFieldId)];

                int increment = 2; //the first two fields are reserved for index and class lables
                for(int i = 0; i < imageInfoFieldCount; i++){ // the remaining are saved after them, skipping the index and class indexes
                    if (i == indexFieldId || i == classFieldId)
                        increment--;
                    else
                        label.at<float>(0, i+increment) = idsToInternalIds.at(i)[info.at(i)];
                }

                path = is.getImagePath();
                cout << path << endl;
                IDataModel* data = analyser->getFeatures(path);
                vector<float>* v = (vector<float>*) data->getValue();
                vector<float> v2 = *v;
                MatrixTools::vectorToMat(v2, featuresRow);

                //cout << featuresRow << endl;
                // add the features to the main feature and label matrix
                features.push_back(featuresRow);
                labels.push_back(label);
            }
		}
	}

	features.convertTo(features,CV_32F);
	labels.convertTo(labels,CV_32F);

	if(parameters.count("split") > 0){
		vector<IClassifier*> classifiers;
		TrainTestFeaturesTools tt (features, labels, classifiers);
		float ratio = atof(parameters["split"].c_str());
		tt.splitDataForTest(ratio);

		Mat featuresTrain = tt.getTrainingData();
		Mat labelsTrain = tt.getTrainingLabels();

		Mat featuresTest = tt.getTestData();
		Mat labelsTest = tt.getTestLabels();

		stringstream ss;

		cout << features.rows << " " << features.cols << " " << labels.rows << " " << labels.cols << endl;
		cout << featuresTrain.rows << " " << featuresTrain.cols << " " << labelsTrain.rows << " " << labelsTrain.cols << endl;
		cout << featuresTest.rows << " " << featuresTest.cols << " " << labelsTest.rows << " " << labelsTest.cols << endl;

		ss << outputLocation << ".train";

		string name = ss.str();
		MatrixTools::writeBinV2(name,featuresTrain,labelsTrain);

		stringstream ss2;
		ss2 << outputLocation << ".test";
		name = ss2.str();
		MatrixTools::writeBinV2(name,featuresTest,labelsTest);
	} else {
		MatrixTools::writeBinV2(outputLocation,features,labels);
	}


	exportLabels(filename,idsToInternalIds);
	/*
	for(int i = 0; i < 8; i++)
	{

		Json::Value tags;
		result["id"] = rand() % 25000;
		result["rank"] = rand() % 10;
		result["path"] = "/some/path/";
		result["title"] = "randomTitle";
		tags["0"] = "tag1";
		tags["1"] = "tag2";
		tags["2"] = "tag3";
		result["tags"] = tags;
		results[i] = result;
	}
	*/
	Json::Value root;
	root["result"] = "ok";
	root["location"] = outputLocation;
	stringstream ss;
	ss << root;
	return ss.str();
}

