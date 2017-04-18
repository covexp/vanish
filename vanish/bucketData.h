#pragma once
#include <vector>

using namespace std;

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

	vector<short int> valueBucketA;
	vector<short int> valueBucketB;
	vector<BucketEntry> finalBucket;
};

