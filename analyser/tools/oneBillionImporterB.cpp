#include "oneBillionImporterB.h"

oneBillionImporterB::oneBillionImporterB(){}
oneBillionImporterB::~oneBillionImporterB(){}

void oneBillionImporterB::readBin(std::string filenamep, int numberOfRows, cv::Mat& features, long long offsetInRows) {

    FILE * out = fopen(filenamep.c_str(), "rb" );

    int dimensions;

    if (fread(&dimensions,sizeof(int),1,out) == 0){
        std::cout << "error: cannot read begininig of file" << std::endl;
        return;
    }


    features.create(numberOfRows,dimensions,CV_32F);

	/* Open file */
    long long offset = (offsetInRows*(dimensions*1+1*4));
    fseek(out,offset,SEEK_SET);
    if( out != NULL ){
		for (int i=0;i<numberOfRows;i++){

        int currDimensions = 0;
  			if (fread(&currDimensions,4,1,out) == 0){
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
          uint aa = 0;
          if (fread((char*)&aa,1,1,out) == 0){
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

void oneBillionImporterB::readTags(std::string file, int numberOfRows, cv::Mat& tags) {

}

void oneBillionImporterB::readBin(std::string filenamep, int numberOfRows, arma::fmat& features, long long offsetInRows) {


    FILE * out = fopen(filenamep.c_str(), "rb" );

    int dimensions;

    if (fread(&dimensions,sizeof(int),1,out) == 0){
        std::cout << "error: cannot read begininig of file" << std::endl;
        return;
    }


    features.set_size(dimensions,numberOfRows);

	/* Open file */
    long long offset = (offsetInRows*(dimensions*1+1*4));
    fseek(out,offset,SEEK_SET);
    if( out != NULL ){
		for (int i=0;i<numberOfRows;i++){

        int currDimensions = 0;
  			if (fread(&currDimensions,4,1,out) == 0){
                std::cout << "error: cannot read middle of file" << std::endl;
                return;
            }
	   		if(currDimensions != dimensions){
          features = arma::fmat();
         std::cout << "error: dims dont match" << std::endl;
          return;
        }

      if( ferror( out ) )      {
        features = arma::fmat();
         std::cout << "error: EOF reached" << std::endl;
          return;
      }

        for (int g = 0; g < dimensions; g++){
          uint aa = 0;
          if (fread((char*)&aa,1,1,out) == 0){
                std::cout << "error: cannot read middle of file" << std::endl;
                return;
          }
        if( ferror( out ) )      {
                  features = arma::fmat();
                    std::cout << "error: EOF reached" << std::endl;
          return;
      }

          features(g,i) = aa;
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
