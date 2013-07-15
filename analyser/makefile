all:

CC=g++
CFLAGS=-c -Wall 
LDFLAGS=-L/home/amourao/myLibs/lib/ -I/home/amourao/myLibs/include/ -lhdf5 -lflann -ljsoncpp -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_photo -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_nonfree -lfftw3f -lfftw3 -lm -lopencv_legacy -lopencv_flann -larmadillo -llapack -lblas -lPocoFoundation -lPocoData -lPocoDataSQLite -lPocoNet -lPocoUtil -lcurl
SOURCES=../dataModel/NTag.cpp ../dataModel/NVector.cpp ../dataModel/NKeypoint.cpp ../dataModel/DataModelController.cpp ../dataModel/NRoi.cpp ./AnalyserDataType.cpp ./FactoryAnalyser.cpp ./Main2.cpp ./nKeypoint/SURFExtractor.cpp ./nRoi/FaceDetection.cpp ./nTag/FaceRecognition.cpp ./nTag/FacialExpressionRecognition.cpp ./nTag/kNNClassifier.cpp ./nTag/SRClassifier.cpp ./nTag/SVMClassifier.cpp ./nVector/EigenExtractor.cpp ./nVector/GaborExtractor.cpp ./nVector/HistogramExtractor.cpp ./nVector/NullExtractor.cpp ./nVector/SegmentedHistogramExtractor.cpp ./sources/CamSource.cpp ./sources/SingleImageSource.cpp ./sources/TextFileSource.cpp ./sources/VideoSource.cpp ./tools/IlluminationCorrectionFilter.cpp ./tools/ImageSegmentator.cpp ./tools/MatrixTools.cpp ./tools/MIRFlickrImporter.cpp ./tools/TestFunctions.cpp ./tools/TrainTestFeaturesTools.cpp ./nVector/lbp.cpp ./nVector/LBPExtractor.cpp ./nVector/CombinedExtractor.cpp ./nVector/LireExtractor.cpp ../commons/factory/Factory.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=main2

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS)  $< $(LDFLAGS) -o $@
    
clean:
	rm -f $(EXECUTABLE) $(OBJECTS)
