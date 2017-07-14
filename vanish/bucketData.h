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

	std::vector<unsigned char> redBucketA;
	std::vector<unsigned char> redBucketB;
	std::vector<BucketEntry> redFinalBucket;

	std::vector<unsigned char> greenBucketA;
	std::vector<unsigned char> greenBucketB;
	std::vector<BucketEntry> greenFinalBucket;

	std::vector<unsigned char> blueBucketA;
	std::vector<unsigned char> blueBucketB;
	std::vector<BucketEntry> blueFinalBucket;
};

