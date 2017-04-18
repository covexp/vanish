// ImageProcessor
// Class to handle the processing of the image sequence

#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <array>

#include "../../cimg/CImg.h"

#include "bucketData.h"

namespace cimg = cimg_library;

class ImageProcessor
{
private:
	int frames;
	int width;
	int height;
	int channels;
	int depth;

	int minVal;
	int maxVal;
	int bucketSize;
	int buckets;

	BucketData *bucketData;

	std::vector<std::string> fileNames;

	int getABucket(int value);
	int getBBucket(int value);

	void inferParameters();

	void initializeData();

public:
	ImageProcessor();
	~ImageProcessor();

	void setFiles(std::vector<std::string> fn);
	void setBucketSize(int newSize);

	void processSequence();
	void countBuckets();
	void refineSolution();
	void findBiggestBucket();
	void createOutput();
};

