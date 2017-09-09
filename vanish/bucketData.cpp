#include "bucketData.h"

BucketData::BucketData(int width, int height, int buckets) 
    : bucketA(width * height * buckets),
      bucketB(width * height * buckets),
      finalBucket(width * height)
{
}

BucketData::~BucketData()
{
}
