#pragma once
#include <vector>

class pixelBucket
{
private:
	int numBuckets;
	std::vector<int> valueA;
	std::vector<int> valueB;
public:
	pixelBucket();
	pixelBucket(int buckets);
	~pixelBucket();
};

