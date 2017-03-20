#pragma once
#include <vector>

using namespace std;

struct BucketEntry
{
	int id;
	bool isABucket;
	int diff;
};

class BucketData
{
private:

public:
	BucketData(int width, int height, int buckets);
	~BucketData();

	vector<int> valueBucketA;
	vector<int> valueBucketB;
	vector<BucketEntry> finalBucket;
};

