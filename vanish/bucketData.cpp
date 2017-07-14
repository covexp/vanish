#include "bucketData.h"

BucketData::BucketData(int width, int height, int buckets) 
	: redBucketA(width * height * buckets), 
	  redBucketB(width * height * buckets), 
	  redFinalBucket(width * height),
	  greenBucketA(width * height * buckets),
	  greenBucketB(width * height * buckets),
	  greenFinalBucket(width * height),
	  blueBucketA(width * height * buckets),
	  blueBucketB(width * height * buckets),
	  blueFinalBucket(width * height)
{
}

BucketData::~BucketData()
{
}
