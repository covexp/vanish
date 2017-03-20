#include "bucketData.h"

BucketData::BucketData(int width, int height, int buckets) 
	: valueBucketA(width * height * buckets), 
	  valueBucketB(width * height * buckets), 
	  finalBucket(width * height)
{
}

BucketData::~BucketData()
{
}
