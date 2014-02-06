#include "tinyImageImporter.h"

void tinyImageImporter::readBin(std::string filenamep, int numberOfRows, cv::Mat& features, long long offsetInRows) {
	float *datap = new float[numberOfRows*dimensions];

	/* Open file */
    FILE * out = fopen(filenamep.c_str(), "rb" );

    long long offset = offsetInRows*dimensions;
    fseek(out,offset,SEEK_SET);
    if( out != NULL ){
		for (int i=0;i<numberOfRows;i++){

			/* get offset into binary file */ //offset to suport reading vectors from an input set of offsets
			//offset = (long long) (selectp[i]-1) * (long long) (sizeof(float) * dimensions);
			/*mexPrintf("ind: %ld offset: %ld\n",i,offset);*/
			/* seek point in file */
			/*  */


			/* do binary read direct into datap */
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

void tinyImageImporter::readTags(std::string file, int numberOfRows, cv::Mat& tags) {

}
