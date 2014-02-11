#include "oneBillionImporter.h"

oneBillionImporter::oneBillionImporter(){}
oneBillionImporter::~oneBillionImporter(){}

void oneBillionImporter::readBin(std::string filenamep, int numberOfRows, cv::Mat& features, long long offsetInRows) {

    FILE * out = fopen(filenamep.c_str(), "rb" );

    int dimensions;
    int currDimensions;

    fread(&dimensions,sizeof(int),1,out);

	float *datap = new float[numberOfRows*dimensions];

	/* Open file */
    long long offset = (offsetInRows*(dimensions+1/*dim at start of each fv*/))*sizeof(float);
    fseek(out,offset,SEEK_SET);
    if( out != NULL ){
		for (int i=0;i<numberOfRows;i++){

			/* do binary read direct into datap */

			fread(&currDimensions,sizeof(int),1,out);
			if(currDimensions != dimensions)
                std::cout << "error" << std::endl;

			fread(&(datap[i*dimensions]),sizeof(float),dimensions,out);
      	}
	    /* Flush buffer and close file */
      	fclose(out);
    }
    else{
     	/* Error */
       	/*   mexPrintf("Error opening file: %s\n",filenamep);*/
    }

    features = cv::Mat(numberOfRows,dimensions,CV_32F);
    for (int i = 0; i < numberOfRows; i++)
		for (int j = 0; j < dimensions; j++)
			features.at<float>(i,j) = datap[i*dimensions+j];//cv::Mat(numberOfRows, dimensions, CV_32FC1, &datap);
}

void oneBillionImporter::readTags(std::string file, int numberOfRows, cv::Mat& tags) {

}
