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
	string inputDirectory;
	string fileStem;
	string fileType;

	int frames;
	int width;
	int height;

	int minVal;
	int maxVal;
	int bucketSize;
	int buckets;

	vector<string> fileNames;

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

