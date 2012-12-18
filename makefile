all:
	g++-4.7 -std=c++11 -Wall -o main *.cpp -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_nonfree -lfftw3f -lfftw3 -lm
