// ImageProcessor
// Class to handle the processing of the image sequence

#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>

#include <CImg.h>

#include "bucket_data.h"

namespace cimg = cimg_library;

typedef std::vector<std::vector<float>> vec2d;

class ImageProcessor {
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

    std::vector<BucketData<unsigned char>> bucketData;

    std::vector<std::string> fileNames;

    int getABucket(int value) const;
    int getBBucket(int value) const;

    void inferParameters();
    void initializeData();

    void printPixelInformation(int x, int y) const;
    void printImageData() const;

    void countBuckets();
    void findBiggestBucket();
    void createFinal() const;

    void firstPass(vec2d& acc, vec2d& total, std::vector<int>& count) const;
    void countFailed(vec2d& acc, std::vector<int>& count,
        std::vector<bool>& cleared, int confFrames,
        int& failed) const;
    void secondPass(vec2d& acc, vec2d& total, std::vector<int>& count,
        std::vector<bool>& cleared) const;
    void drawImages(vec2d& acc, vec2d& total, std::vector<int> count,
        int confFrames, int& firstPassFail,
        int& secondPassFail) const;

public:
    ImageProcessor();
    ~ImageProcessor();

    void setFiles(const std::vector<std::string>& fn);
    void setBucketSize(int newSize);
    void setConfidenceLevel(float newConf);
    void processSequence();
};
