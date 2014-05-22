#include "GetObjectFeatures.h"


//USAGE: http://localhost:9090/getObjectFeatures?url=file:///localstore/novaemotionData/p01/0000.png&detector=FaceDetection&extractor=GaborFace
static GetObjectFeatures GetObjectFeaturesEndpointFactory;

GetObjectFeatures::GetObjectFeatures(string type){
  this->type = type;
}

GetObjectFeatures::GetObjectFeatures(){

	FactoryEndpoint::getInstance()->registerType("/getObjectFeatures",this);

}

GetObjectFeatures::~GetObjectFeatures()
{}

void* GetObjectFeatures::createType(string& type){
	//TODO
	std::cout << "New type requested: " << type << std::endl;

	if (type == "/getObjectFeatures")
		return new GetObjectFeatures(type);

	std::cerr << "Error registering type from constructor (this should never happen)" << std::endl;
	return NULL;
}


void GetObjectFeatures::handleRequest(string method, map<string, string> queryStrings, istream& input, HTTPServerResponse& resp)
{
  	if(method != "GET"){
		resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
		resp.send();
		return ;
	}

	resp.setContentType("application/json");
	std::ostream& out = resp.send();

	std::string response("");

	if (type == "/getObjectFeatures")
		response = getFeatures(queryStrings);

	//std::cout << response << std::endl;
	out << response;
	out.flush();


}

string GetObjectFeatures::indexFeatures(map<string, string > parameters){

	FileDownloader fd;

	string filename = fd.getFile(parameters["url"]);
	string extractorName = parameters["extractor"];


	FactoryAnalyser * f = FactoryAnalyser::getInstance();

	//f->listTypes();



	IAnalyser* extractor= (IAnalyser*)f->createType(extractorName);

	IDataModel* data = extractor->getFeatures(filename);
	vector<float>* features = (vector<float>*) data->getValue();

	//for (int i = 0; i < features->size(); i++){
	//	cout << features->at(i) << " " ;
	//}
	//cout << endl;

	Json::Value root;
	Json::Value results;

	Json::Value featureArray(Json::arrayValue);
	for (uint i = 0; i < features->size(); i++){

		featureArray.append(features->at(i));
		//cout << features->at(i) << " " ;
	}
	/*
	for(int i = 0; i < 8; i++)
	{
		Json::Value result;
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
	root["result"] = "ok";
	stringstream ss;
	ss << root;
	return ss.str();
}

string GetObjectFeatures::getFeatures(map<string, string > parameters){

	FileDownloader fd;

	string filename = fd.getFile(parameters["url"]);
	string extractorName = parameters["extractor"];
	string detectorName = parameters["detector"];


	FactoryAnalyser * f = FactoryAnalyser::getInstance();


	IAnalyser* detector= (IAnalyser*)f->createType(detectorName);
	IDataModel* objectsLocationGen = detector->getFeatures(filename);
	map<string,region>* objectsLocation = (map<string,region>*) objectsLocationGen->getValue();



	FeatureExtractor* extractor= (FeatureExtractor*)f->createType(extractorName);
	Json::Value root;
	Json::Value featuresJ;

	cv::Mat image = cv::imread(filename);
	std::map<string,region>::iterator iter;

    for (iter = objectsLocation->begin(); iter != objectsLocation->end(); ++iter) {


        region r = iter->second;

        cv::Rect roi (r.x,r.y,r.width,r.height);

        cv::Mat object (image,roi);

		unsigned found = filename.find_last_of(".");


		stringstream ss;
		ss << filename.substr(0,found) << "_" <<  iter->first << filename.substr(found);
		string newFilename = ss.str();

        cv::imwrite(newFilename,object);

		IDataModel* featuresGen = extractor->getFeatures(newFilename);
		vector<float>* features = (vector<float>*) featuresGen->getValue();


		//for (int i = 0; i < features->size(); i++){
		//	cout << features->at(i) << " " ;
		//}
		//cout << endl;

		Json::Value featureArray(Json::arrayValue);
		for (uint i = 0; i < features->size(); i++){
			featureArray.append(features->at(i));
		}

		featuresJ[iter->first] = featureArray;
    }
    /*
    Mat image2;

    cv::cvtColor(image, image2, COLOR_RGB2GRAY);
    image2.convertTo(image2,CV_32F);

    string binPath = "./tmpData/features.bin";

    MatrixTools::writeBin(binPath,image2,image2);

    Mat image3;
    Mat labels3;

    MatrixTools::readBin(binPath,image3,labels3);

	image2 /=255.0;
	image3 /=255.0;

    cv::imshow("a",image2);
    cv::imshow("b",image3);

    cv::waitKey();
    */

	root["features"] = featuresJ;

	stringstream ss;
	ss << root;
	return ss.str();
}
