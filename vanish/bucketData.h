#pragma once
#include <vector>

struct BucketEntry
{
    unsigned char id;
    bool isABucket;
    short int diff;
};

class BucketData
{
private:

public:
    BucketData(int width, int height, int buckets);
    ~BucketData();

    std::vector<unsigned char> bucketA;
    std::vector<unsigned char> bucketB;
    std::vector<BucketEntry> finalBucket;
};

