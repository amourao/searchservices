#include "oneBillionImporter.h"

oneBillionImporter::oneBillionImporter(){}
oneBillionImporter::~oneBillionImporter(){}

void oneBillionImporter::readBin(std::string filenamep, int numberOfRows, cv::Mat& features, long long offsetInRows) {

    FILE * out = fopen(filenamep.c_str(), "rb" );

    int dimensions;

    if (fread(&dimensions,sizeof(int),1,out) == 0){
        std::cout << "error: cannot read begininig of file" << std::endl;
        return;
    }


    features = cv::Mat(numberOfRows,dimensions,CV_32F);

	/* Open file */
    long long offset = (offsetInRows*(dimensions+1/*dim at start of each fv*/))*sizeof(float);
    fseek(out,offset,SEEK_SET);
    if( out != NULL ){
		for (int i=0;i<numberOfRows;i++){

        int currDimensions = 0;
  			if (fread(&currDimensions,sizeof(int),1,out) == 0){
                std::cout << "error: cannot read middle of file" << std::endl;
                return;
            }
	   		if(currDimensions != dimensions){
          features = cv::Mat();
         std::cout << "error: dims dont match" << std::endl;
          return;
        }

      if( ferror( out ) )      {
        features = cv::Mat();
         std::cout << "error: EOF reached" << std::endl;
          return;
      }

        for (int g = 0; g < dimensions; g++){
          float aa = 0;
          if (fread(&aa,sizeof(float),1,out) == 0){
                std::cout << "error: cannot read middle of file" << std::endl;
                return;
          }
        if( ferror( out ) )      {
                  features = cv::Mat();
                    std::cout << "error: EOF reached" << std::endl;
          return;
      }

          features.at<float>(i,g) = aa;//cv::Mat(numberOfRows, dimensions, CV_32FC1, &datap);
        }
    }
	    /* Flush buffer and close file */
      	fclose(out);
    }
    else{
     	/* Error */
       	/*   mexPrintf("Error opening file: %s\n",filenamep);*/
    }
}

void oneBillionImporter::readTags(std::string file, int numberOfRows, cv::Mat& tags) {

}
