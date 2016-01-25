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
          unsigned char aa = 0;
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

void oneBillionImporterB::readBinSlow(std::string filenamep, int numberOfRows, arma::Mat<uchar>& features, long long offsetInRows) {


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
                features = arma::Mat<uchar>();
                std::cout << "error: dims dont match" << std::endl;
                return;
        }

      if( ferror( out ) )      {
        features = arma::Mat<uchar>();
         std::cout << "error: EOF reached" << std::endl;
          return;
      }

        for (int g = 0; g < dimensions; g++){
          unsigned char aa = 0;
          if (fread((char*)&aa,1,1,out) == 0){
                std::cout << "error: cannot read middle of file" << std::endl;
                return;
          }
        if( ferror( out ) )      {
                  features = arma::Mat<uchar>();
                    std::cout << "error: EOF reached" << std::endl;
          return;
      }
            //std::cout << aa << " " << (unsigned int)aa << std::endl;
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
          unsigned char aa = 0;
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


void oneBillionImporterB::readBin(std::string filenamep, int numberOfRows, arma::Mat<uchar>& features, long long offsetInRows) {


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
                features = arma::Mat<uchar>();
                std::cout << "error: dims dont match" << std::endl;
                return;
        }

      if( ferror( out ) )      {
        features = arma::Mat<uchar>();
         std::cout << "error: EOF reached" << std::endl;
          return;
      }
        //for (int g = 0; g < dimensions; g++){
          uchar* aa = new uchar[dimensions];
          if (fread(aa,1,dimensions,out) == 0){
                std::cout << "error: cannot read middle of file" << std::endl;
                return;
          }
        if( ferror( out ) )      {
                  features = arma::Mat<uchar>();
                    std::cout << "error: EOF reached" << std::endl;
          return;
      }
            //std::cout << aa << " " << (unsigned int)aa << std::endl;
            memcpy(&features(0,i),aa,dimensions);
        //}
    }
	    /* Flush buffer and close file */
      	fclose(out);
    }
    else{
     	/* Error */
       	/*   mexPrintf("Error opening file: %s\n",filenamep);*/
    }

}


void oneBillionImporterB::readBin(std::string filenamep, arma::Mat<int>& features, std::vector<uint>& buckets) {
    std::cout << "error:  not implemented int" << std::endl;
}

void oneBillionImporterB::readBin(std::string filenamep, arma::Mat<float>& features, std::vector<uint>& buckets) {
    std::cout << "error:  not implemented float" << std::endl;
}

void oneBillionImporterB::readBin(std::string filenamep, arma::Mat<uchar>& features, std::vector<uint>& buckets) {

    FILE * out = fopen(filenamep.c_str(), "rb" );

    uint dimensions = 0;
    uint currDimensions = 0;

    if (fread(&dimensions,sizeof(int),1,out) == 0){
        std::cout << "error: cannot read begininig of file" << std::endl;
        return;
    }
    fseek(out,0,SEEK_SET);

    features.set_size(dimensions,buckets.size());

    std::cout << "Reading " << dimensions << " dims for " << buckets.size() << " buckets" << std::endl;
	/* Open file */
	for (uint i=0;i<buckets.size();i++){
        fseek(out,0,SEEK_SET);
        uint bucket = buckets[i];

        unsigned long long offset = (bucket*(dimensions+4));
        std::cout << (bucket*(dimensions+4)) << std::endl;
        std::cout << (dimensions+4) << std::endl;
        std::cout << offset << std::endl;
        fseek(out,offset,SEEK_SET);
        if( out != NULL ){

  			if (fread(&currDimensions,1,sizeof(int),out) == 0){
                std::cout << "error: cannot read middle of file" << std::endl;
                return;
            }
	   		if(currDimensions != dimensions){
                features = arma::Mat<uchar>();
                std::cout << "error: dims dont match at bucket " << bucket << " offset " << offset << " seek "<< ftell(out) << ": " << currDimensions << " " << dimensions << std::endl;
                return;
        }

      if( ferror( out ) )      {
        features = arma::Mat<uchar>();
         std::cout << "error: EOF reached" << std::endl;
          return;
      }
        //for (int g = 0; g < dimensions; g++){
          uchar* aa = new uchar[dimensions];
          if (fread(aa,1,dimensions,out) == 0){
                std::cout << "error: cannot read middle of file" << std::endl;
                return;
          }
        if( ferror( out ) )      {
                  features = arma::Mat<uchar>();
                    std::cout << "error: EOF reached" << std::endl;
          return;
      }
            //std::cout << aa << " " << (unsigned int)aa << std::endl;
            memcpy(&features(0,i),aa,dimensions);
            std::cout << features(0,i) << std::endl;
        //}
    }
	    /* Flush buffer and close file */
    }
          	fclose(out);


}

