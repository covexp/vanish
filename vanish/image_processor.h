// ImageProcessor
// Class to handle the processing of the image sequence

#pragma once

#include <string>
#include <vector>

#include "bucket_data.h"

class ImageProcessor {
public:
    ImageProcessor();
    ~ImageProcessor();

    void setFiles(const std::vector<std::string>& fn);
    void setBucketSize(int newSize);
    void setBitDepth(int newDepth);
    void setConfidenceLevel(float newConf);
    void processSequence();

private:
    using vec2d = std::vector<std::vector<float>>;
    using BucketType = unsigned char;

    std::vector<BucketData<BucketType>> bucketData;
    std::vector<std::string> fileNames;

    int getABucket(int value) const;
    int getBBucket(int value) const;

    void inferParameters();
    void initializeData();

    void printPixelInformation(int x, int y) const;
    void printImageData() const;

    void countBuckets();
    void findBiggestBucket();
    void createFinal();

    void firstPass();
    void countFailed(int confFrames, int& failed);
    void secondPass();
    void drawImages(int confFrames, int& firstPassFail, int& secondPassFail);

    int _frames = 0;
    int _width = 0;
    int _height = 0;
    int _size = 0;
    int _channels = 0;
    int _depth = 0;

    int _minVal = 0;
    int _maxVal = 0;
    int _bucketSize = 0;
    int _buckets = 0;
    int _bitDepth = 0;

    float _confLevel = 0.0f;

    std::vector<std::vector<float>> _acc;
    std::vector<std::vector<float>> _total;
    std::vector<int> _count;
    std::vector<bool> _cleared;
};