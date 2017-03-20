// ImageProcessor
// Class to handle the processing of the image sequence

#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <array>

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
	int frames;
	int width;
	int height;

	int minVal;
	int maxVal;
	int bucketSize;
	int buckets;

	vector<string> fileNames;

	int getABucket(int value);
	int getBBucket(int value);

public:
	ImageProcessor();
	~ImageProcessor();

	void setFiles(vector<string> fn);
	void setBucketSize(int newSize);

	void processSequence();
};

