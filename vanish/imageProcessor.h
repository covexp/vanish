// ImageProcessor
// Class to handle the processing of the image sequence

#pragma once

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <array>

#include <CImg.h>

#include "bucketData.h"

namespace cimg = cimg_library;

class ImageProcessor
{
private:
    int frames;
    int width;
    int height;
    int size;
    int channels;
    int depth;

    int minVal;
    int maxVal;
    int bucketSize;
    int buckets;

    float confLevel;

    std::vector<BucketData *> bucketData;

    std::vector<std::string> fileNames;

    int getABucket(int value);
    int getBBucket(int value);

    void inferParameters();

    void initializeData();

    void printPixelInformation(int x, int y);
    void printImageData();
public:
    ImageProcessor();
    ~ImageProcessor();

    void setFiles(std::vector<std::string> fn);
    void setBucketSize(int newSize);
    void setConfidenceLevel(float newConf);

    void processSequence();
    void countBuckets();
    void refineSolution();
    void findBiggestBucket();
    void createFinal();
};
