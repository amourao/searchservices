#pragma once

#include <opencv2/opencv.hpp>
#include <numeric>
#include <fstream>

using namespace std;
using namespace cv;

class ShotDetector {
public:
	ShotDetector();
	~ShotDetector();

	void detectScenes(const string filename, int step, vector<int>& diffs, vector<int>& frames);
	void detectScenesV1(const string filename, int step, vector<int>& diffs, vector<int>& frames);

	void getPeaks(const int minSceneDurationFrames,const int numStdDevs, const vector<int>& diffs, const vector<int>& frames, vector<int>& keyframes, vector<int>& keyframesDiffs);
	void addMiddleKeyframes(const vector<int>& keyframes, vector<int>& newKeyframes);
	void writeFrames(const string filename, const vector<int>& frames, vector<string>& framesPaths);
	void writeFramesV1(const string filename, const vector<int>& frames, vector<string>& framesPaths);
	void convertFramesIndexToTimes(const string filename,const vector<int>& frames, vector<double>& times);

	static void processOneVideo(string filename, int step, int numStdDev, int minSceneTime, bool saveFrames, bool saveOutput);

};
