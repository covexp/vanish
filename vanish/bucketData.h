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

	std::vector<short int> valueBucketA;
	std::vector<short int> valueBucketB;
	std::vector<BucketEntry> finalBucket;
};

