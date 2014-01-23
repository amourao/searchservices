all:

CC=g++-4.6 
CFLAGS=-c -Wall 
LDFLAGS=-L/home/amourao/myLibs/lib/ -I/home/amourao/myLibs/include/ -lhdf5 -lflann -ljsoncpp -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_photo -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_nonfree -lfftw3f -lfftw3 -lm -lopencv_legacy -lopencv_flann -larmadillo -llapack -lblas -lPocoFoundation -lPocoData -lPocoDataSQLite -lPocoNet -lPocoXML -lPocoJSON -lPocoUtil -lcurl -lvw
SOURCES=./rest/Endpoints/TestEndpoint.cpp  ./rest/FactoryEndpoint.cpp  ./rest/main.cpp ./rest/RestRequestHandler.cpp ./rest/RestServer.cpp ./commons/FileDownloader.cpp ./commons/factory/Factory.cpp ./dataModel/NTag.cpp ./dataModel/NVector.cpp ./dataModel/NKeypoint.cpp ./dataModel/DataModelController.cpp ./dataModel/NRoi.cpp ./analyser/FactoryAnalyser.cpp ./analyser/FactoryClassifier.cpp ./analyser/Main.cpp ./analyser/nKeypoint/SURFExtractor.cpp ./analyser/nRoi/FaceDetection.cpp ./analyser/nTag/FaceRecognition.cpp ./analyser/nTag/FacialExpressionRecognition.cpp ./analyser/nTag/kNNClassifier.cpp ./analyser/nTag/SRClassifier.cpp ./analyser/nTag/SVMClassifier.cpp ./analyser/nTag/VWBasicClassifier.cpp ./analyser/nVector/EigenExtractor.cpp ./analyser/nVector/GaborExtractor.cpp ./analyser/nVector/HistogramExtractor.cpp ./analyser/nVector/NullExtractor.cpp ./analyser/nVector/SegmentedHistogramExtractor.cpp ./analyser/sources/CamSource.cpp ./analyser/sources/SingleImageSource.cpp ./analyser/sources/TextFileSource.cpp ./analyser/sources/VideoSource.cpp ./analyser/tools/IlluminationCorrectionFilter.cpp ./analyser/tools/ImageSegmentator.cpp ./analyser/tools/MatrixTools.cpp ./analyser/tools/MIRFlickrImporter.cpp ./analyser/tools/TestFunctions.cpp ./analyser/tools/TrainTestFeaturesTools.cpp ./rest/Endpoints/analyser/ExtractFeatures.cpp ./rest/Endpoints/searchDemo/populateSearchDemo.cpp ./analyser/nVector/lbp.cpp ./analyser/nVector/LBPExtractor.cpp ./rest/Endpoints/analyser/MedicalSearchEngine.cpp ./analyser/nVector/CombinedExtractor.cpp ./analyser/nVector/LireExtractor.cpp ./rest/Endpoints/analyser/SearchResult.cpp ./rest/Endpoints/analyser/CombSearchResult.cpp ./rest/Endpoints/analyser/GetObjectFeatures.cpp ./rest/Endpoints/analyser/EnsembleClassifier.cpp ./rest/Endpoints/analyser/GenericClassifier.cpp ./indexer/FactoryIndexer.cpp ./indexer/LinearkNNIndexer.cpp ./commons/StringTools.cpp ./rest/Endpoints/indexer/GenericIndexer.cpp ./analyser/nVector/GISTExtractor.cpp
OBJECTS=./analyser/nVector/gist.o ./analyser/nVector/standalone_image.o $(SOURCES:.cpp=.o) 
 
EXECUTABLE=server

ANALYSERSOURCES=./rest/Endpoints/TestEndpoint.cpp  ./rest/FactoryEndpoint.cpp  ./analyser/Main2.cpp ./rest/RestRequestHandler.cpp ./rest/RestServer.cpp ./commons/FileDownloader.cpp ./commons/factory/Factory.cpp ./dataModel/NTag.cpp ./dataModel/NVector.cpp ./dataModel/NKeypoint.cpp ./dataModel/DataModelController.cpp ./dataModel/NRoi.cpp ./analyser/FactoryAnalyser.cpp ./analyser/FactoryClassifier.cpp ./analyser/Main.cpp ./analyser/nKeypoint/SURFExtractor.cpp ./analyser/nRoi/FaceDetection.cpp ./analyser/nTag/FaceRecognition.cpp ./analyser/nTag/FacialExpressionRecognition.cpp ./analyser/nTag/kNNClassifier.cpp ./analyser/nTag/SRClassifier.cpp ./analyser/nTag/SVMClassifier.cpp ./analyser/nTag/VWBasicClassifier.cpp ./analyser/nVector/EigenExtractor.cpp ./analyser/nVector/GaborExtractor.cpp ./analyser/nVector/HistogramExtractor.cpp ./analyser/nVector/NullExtractor.cpp ./analyser/nVector/SegmentedHistogramExtractor.cpp ./analyser/sources/CamSource.cpp ./analyser/sources/SingleImageSource.cpp ./analyser/sources/TextFileSource.cpp ./analyser/sources/VideoSource.cpp ./analyser/tools/IlluminationCorrectionFilter.cpp ./analyser/tools/ImageSegmentator.cpp ./analyser/tools/MatrixTools.cpp ./analyser/tools/MIRFlickrImporter.cpp ./analyser/tools/TestFunctions.cpp ./analyser/tools/TrainTestFeaturesTools.cpp ./rest/Endpoints/analyser/ExtractFeatures.cpp ./rest/Endpoints/searchDemo/populateSearchDemo.cpp ./analyser/nVector/lbp.cpp ./analyser/nVector/LBPExtractor.cpp ./rest/Endpoints/analyser/MedicalSearchEngine.cpp ./analyser/nVector/CombinedExtractor.cpp ./analyser/nVector/LireExtractor.cpp ./rest/Endpoints/analyser/SearchResult.cpp ./rest/Endpoints/analyser/CombSearchResult.cpp ./rest/Endpoints/analyser/GetObjectFeatures.cpp ./rest/Endpoints/analyser/EnsembleClassifier.cpp ./rest/Endpoints/analyser/GenericClassifier.cpp ./indexer/FactoryIndexer.cpp ./indexer/LinearkNNIndexer.cpp ./commons/StringTools.cpp ./rest/Endpoints/indexer/GenericIndexer.cpp ./analyser/nVector/GISTExtractor.cpp
ANALYSEROBJECTS=./analyser/nVector/gist.o ./analyser/nVector/standalone_image.o $(ANALYSERSOURCES:.cpp=.o)
ANALYSEREXECUTABLE=runAnalyser

.PHONY: all
all: .make_gist serverMain analyserMain 


serverMain: $(SOURCES) $(EXECUTABLE)
analyserMain: $(ANALYSERSOURCES) $(ANALYSEREXECUTABLE)

.make_gist:
	gcc -c -Wall -o ./analyser/nVector/gist.o -g ./analyser/nVector/gist.c  -DUSE_GIST -DSTANDALONE_GIST
	gcc -c -Wall -o ./analyser/nVector/standalone_image.o -g ./analyser/nVector/standalone_image.c  -DUSE_GIST -DSTANDALONE_GIST

	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(ANALYSEREXECUTABLE): $(ANALYSEROBJECTS) 
	$(CC) $(ANALYSEROBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS)  $< $(LDFLAGS) -o $@
    
clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
