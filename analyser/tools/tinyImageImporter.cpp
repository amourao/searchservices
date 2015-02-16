#include "tinyImageImporter.h"


tinyImageImporter::tinyImageImporter(){}
tinyImageImporter::~tinyImageImporter(){}

void tinyImageImporter::readBin(std::string filenamep, int numberOfRows, cv::Mat& features, long long offsetInRows) {

	features = cv::Mat(numberOfRows,dimensions,CV_32F);
	float *datap = new float[1*dimensions];

	/* Open file */
    FILE * out = fopen(filenamep.c_str(), "rb" );

    long long offset = offsetInRows*dimensions*sizeof(float);
    fseek(out,offset,SEEK_SET);
    if( out != NULL ){
		for (int i=0;i<numberOfRows;i++){

			/* do binary read direct into datap */
			if (fread(&(datap[0]),sizeof(float),dimensions,out) == 0){
                std::cout << "error: cannot read middle of file" << std::endl;
                return;
            } else {
                for (int j=0;j<dimensions;j++){
                    features.at<float>(i,j) = datap[j];
                }
            }
      	}
	    /* Flush buffer and close file */
      	fclose(out);
    }
    else{
     	/* Error */
       	/*   mexPrintf("Error opening file: %s\n",filenamep);*/
    }

    delete[] datap;
}

void tinyImageImporter::readTags(std::string file, int numberOfRows, cv::Mat& tags) {

}
