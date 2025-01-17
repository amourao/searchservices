#include "ShotDetector.h"


ShotDetector::ShotDetector(){
}


ShotDetector::~ShotDetector(){

}

void ShotDetector::detectScenesV1(const string filename,const int step, vector<int>& diffs, vector<int>& frames){
	VideoCapture capture = VideoCapture(filename);
	if(!capture.isOpened()){
		cout << "Capture from video " << filename <<  " didn't work"<< endl;
		return;
    }

	Mat frame, lastFrame;
	capture >> frame;
	frame.copyTo(lastFrame);

	int frameTotal = capture.get(CV_CAP_PROP_FRAME_COUNT);

	int frameC = step;
	while(frameC < frameTotal){

		capture.set(CV_CAP_PROP_POS_FRAMES,frameC);
		capture >> frame;

        Mat diff;
        absdiff(lastFrame, frame, diff);
        Scalar s = mean(diff);
        diffs.push_back((s[0] + s[1] + s[2]));
        frames.push_back(frameC);

        frame.copyTo(lastFrame);
        frameC+=step;
	}
}

void ShotDetector::detectScenes(const string filename,const  int step, vector<int>& diffs, vector<int>& frames){
	VideoCapture capture = VideoCapture(filename);
	if(!capture.isOpened()){
		cout << "Capture from video " << filename <<  " didn't work"<< endl;
		return;
    }

	Mat frame, lastFrame;
	capture >> frame;
	frame.copyTo(lastFrame);

	int frameC = 0;
	while(!frame.empty()){

		if ((frameC % step) == 0){

			Mat diff;
			absdiff(lastFrame, frame, diff);
			Scalar s = mean(diff);
			diffs.push_back((s[0] + s[1] + s[2]));
			frames.push_back(frameC);

			frame.copyTo(lastFrame);
		}
		frameC++;
		capture >> frame;
	}
}

void ShotDetector::convertFramesIndexToTimes(const string filename,const vector<int>& frames, vector<double>& times){
	VideoCapture capture = VideoCapture(filename);
	if(!capture.isOpened()){
		cout << "Capture from video " << filename <<  " didn't work"<< endl;
		return;
    }
    double fps = capture.get(CV_CAP_PROP_FPS);
    for(int frame: frames)
        times.push_back(frame/fps);
}


void ShotDetector::getPeaks(const int minSceneDurationFrames,const int numStdDevs, const vector<int>& diffs, const vector<int>& frames, vector<int>& keyframes, vector<int>& keyframesDiffs){

	double mean = std::accumulate(diffs.begin(), diffs.end(), 0.0) / diffs.size();

	double stdDev = 0;

	for(uint i = 0; i < diffs.size(); i++){
		stdDev += pow(diffs.at(i)-mean,2);
	}

	stdDev = sqrt(stdDev/diffs.size());

	double threshold = mean + (numStdDevs * stdDev);



	double lastDiff = -1;
	double lastFrame = -1;

	double lastDiff2 = -1;

	keyframes.push_back(0);
	keyframesDiffs.push_back(0);
	double currentSceneStart = 0;
	for(uint i = 0; i < diffs.size(); i++){
		double currDiff = diffs.at(i);
		double currFrame = frames.at(i);
		double currDuration = currFrame - currentSceneStart;

		if(lastDiff2 < lastDiff && currDiff < lastDiff && lastDiff > threshold && mean > 0.5 && stdDev > 0.5 && currDuration > minSceneDurationFrames){
			keyframes.push_back(lastFrame);
			keyframesDiffs.push_back(lastDiff);
		}

		if(i > 1){
			lastDiff2 = lastDiff;
		}
		if(i > 0){
			lastDiff = currDiff;
			lastFrame = currFrame;
		}

	}
	keyframes.push_back(frames.at(frames.size()-1));
	keyframesDiffs.push_back(diffs.at(diffs.size()-1));
}

void ShotDetector::addMiddleKeyframes(const vector<int>& keyframes, vector<int>& newKeyframes){

	for(uint i = 0; i < keyframes.size()-1; i++){
		double currFrame = keyframes.at(i);
		double nextFrame = keyframes.at(i+1);

		newKeyframes.push_back(currFrame);
		newKeyframes.push_back((nextFrame+currFrame)/2);
	}
	newKeyframes.push_back(keyframes.at(keyframes.size()-1));
}

void ShotDetector::writeFrames(const string filename, const vector<int>& frames,const bool saveMiddleOnly, vector<string>& framesPaths){
	VideoCapture capture = VideoCapture(filename);
	if(!capture.isOpened()){
		cout << "Capture from video " << filename <<  " didn't work"<< endl;
		return;
	}

	Mat frame;

	for(uint frameC = 0; frameC < frames.size(); frameC++){
        if (saveMiddleOnly){
            frameC++;
            if (frameC == frames.size())
                continue;
        }
        int frameIndex = frames.at(frameC);
		capture.set(CV_CAP_PROP_POS_FRAMES,frameIndex);
		capture >> frame;
		stringstream ss;
		ss << filename << "." << frameIndex << ".png";
		framesPaths.push_back(ss.str());
		imwrite(ss.str(),frame);
	}
}

void ShotDetector::writeFramesV1(const string filename, const vector<int>& frames, vector<string>& framesPaths){
	VideoCapture capture = VideoCapture(filename);
	if(!capture.isOpened()){
		cout << "Capture from video " << filename <<  " didn't work"<< endl;
		return;
	}

	Mat frame;
	capture >> frame;

	int frameC = 0;
	uint frameIndex = 0;
	while(!frame.empty() && frameIndex < frames.size()){

		if (frames.at(frameIndex) == frameC){
			stringstream ss;
			ss << filename << "." << frameIndex << ".png";
			framesPaths.push_back(ss.str());
			imwrite(ss.str(),frame);
			frameIndex++;
		}
		frameC++;
		capture >> frame;
	}
}

void ShotDetector::processOneVideo(string filename, int step, int numStdDev, int minSceneTime, bool saveFrames, bool saveOutput, bool saveMiddleOnly){
	vector<int> diffs,frames,keyframes,keyframesWithMiddle, keyframesDiffs;

	vector<double> framesTimes;
	vector<string> framesPaths;

	ShotDetector s;
	s.detectScenes(filename,step,diffs,frames);
	s.getPeaks(minSceneTime,numStdDev,diffs,frames,keyframes,keyframesDiffs);
	s.addMiddleKeyframes(keyframes,keyframesWithMiddle);
	s.convertFramesIndexToTimes(filename,keyframesWithMiddle,framesTimes);

	if(saveFrames)
        s.writeFrames(filename,keyframesWithMiddle,saveMiddleOnly,framesPaths);

    if (saveOutput){
        string csvFile = filename + ".csv";
        ofstream f(csvFile.c_str());
        /*
        for(uint i = 0; i < diffs.size(); i++){
            f << frames.at(i) << ";" << diffs.at(i);
            if(i < keyframesWithMiddle.size())
                f << ";;" << keyframesWithMiddle.at(i);
            f << endl;
        }
        */
        bool first = true;
        for(uint i = 0; i < keyframesWithMiddle.size(); i++){
            f << ";" << keyframesWithMiddle.at(i) << ";" << framesTimes.at(i) << ";";

            if (i % 2 == 0 && !first){
                f << endl;
                if (i+1 == keyframesWithMiddle.size())
                    continue;
                i--;
                first = true;
            } else {
                first = false;
            }
        }
        f.close();
	}
}
