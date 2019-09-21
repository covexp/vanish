// BucketData
// Holds the bucket data used by the image processor

#pragma once

#include <vector>

template <typename T>
struct BucketEntry {
    T id;
    bool isABucket;
    short int diff;
};

template <class T>
class BucketData {
private:
public:
    BucketData(int width, int height, int buckets)
        : bucketA(width * height * buckets)
        , bucketB(width * height * buckets)
        , finalBucket(width * height)
    {
    }

    ~BucketData() {}

    std::vector<T> bucketA;
    std::vector<T> bucketB;
    std::vector<BucketEntry<T>> finalBucket;
};
