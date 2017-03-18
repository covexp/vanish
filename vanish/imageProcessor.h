// ImageProcessor
// Class to handle the processing of the image sequence

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "../../cimg/CImg.h"

using namespace std;
using namespace cimg_library;

struct BucketEntry
{
	int id;
	bool isABucket;
};

class ImageProcessor
{
private:
	std::string inputDirectory;
	std::string fileStem;
	std::string fileType;

	int frames;
	int width;
	int height;

	int minVal;
	int maxVal;
	int bucketSize;
	int buckets;


	//const int MINVAL = 0;
	//const int MAXVAL = 255;
	//const int WIDTH = 480;
	//const int HEIGHT = 480;
	//const int FRAMES = 67;
	//const int BUCKETSIZE = 8;
	//const int BUCKETS = (MAXVAL + 1) / BUCKETSIZE;

	//const string DEFAULT_DIRECTORY = "input";
	//const string FILESTEM = "picpick";
	//const string FILETYPE = ".png";


public:
	ImageProcessor();
	~ImageProcessor();
	void setFrames(int newFrames);
	void setInputDirectory(string newDir);
	void setBucketSize(int newSize);
	int getABucket(int value);
	int getBBucket(int value);
	void processSequence();
};

