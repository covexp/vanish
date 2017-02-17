// Vanish
// Remove transient objects from an image sequence

#include <iostream>
#include <string>
#include "../../cimg/CImg.h"

using namespace cimg_library;
using namespace std;

const int MINVAL = 0;
const int MAXVAL = 255;
const int WIDTH = 480;
const int HEIGHT = 480;
const int FRAMES = 10;
const int BUCKETSIZE = 8;
const int BUCKETS = (MAXVAL + 1) / BUCKETSIZE;

const string DIRECTORY = "logi";
const string FILESTEM = "salsa (";
const string FILETYPE = ").jpg";

struct bucketEntry
{
	int id;
	bool isABucket;
};

int getABucket(int value)
{
	if (value < MINVAL)
		return 0;

	if (value > MAXVAL)
		return BUCKETS - 1;

	return value / BUCKETSIZE;
}

int getBBucket(int value)
{
	return getABucket(value + (BUCKETSIZE / 2));
}

int main(void)
{
	cout << "Vanish - Version 0.01" << endl;

	CImg<unsigned char> visu(WIDTH, HEIGHT, 1, 1, 0);
	CImgDisplay main_disp(WIDTH, HEIGHT, "Reconstructed background");

	// Buckets store the number of times in the image sequence that the pixel has a value that falls in the given range, or "bucket"
	// A-buckets span the whole range from 0 to 255
	// B-buckets are offset from A-buckets by half of the bucket size and cover a subset of the whole color range
	//
	// The motivation for having two sets of buckets are cases when the pixel values cluster around a bucket boundary and would be split in two.
	// Having an additional set of buckets which are centered around these boundaries helps catch these values and keeps them together.
	int *valueBucketA = new int[WIDTH * HEIGHT * BUCKETS]();
	int *valueBucketB = new int[WIDTH * HEIGHT * BUCKETS]();
	bucketEntry *finalBucket = new bucketEntry[WIDTH * HEIGHT];

	// Read image frames and count the buckets
	for (int frame = 0; frame < FRAMES; frame++)
	{
		string filename = DIRECTORY + "/" + FILESTEM + to_string(1 + frame) + FILETYPE;
		CImg<unsigned char> newImage(filename.c_str());

		cout << "|";

		for (int i = 0; i < WIDTH; i++)
		{
			for (int j = 0; j < HEIGHT; j++)
			{
				int pixel = newImage(i, j, 0, 0);

				int a_bucket = getABucket(pixel);
				valueBucketA[i + j * WIDTH + a_bucket * (WIDTH * HEIGHT)]++;

				int b_bucket = getBBucket(pixel);
				valueBucketB[i + j * WIDTH + b_bucket * (WIDTH * HEIGHT)]++;

				visu(i, j) = 127;
			}
		}
	}

	// Find the biggest bucket
	for (int i = 0; i < WIDTH; i++)
	{
		for (int j = 0; j < HEIGHT; j++)
		{
			int maxCount = 0;
			int maxBucket = -1;
			bool maxTypeA = true;

			for (int bucket = 0; bucket < BUCKETS; bucket++)
			{
				if (valueBucketA[i + j * WIDTH + bucket * (WIDTH * HEIGHT)] > maxCount)
				{
					maxCount = valueBucketA[i + j * WIDTH + bucket * (WIDTH * HEIGHT)];
					maxBucket = bucket;
					maxTypeA = true;
				}
				if (valueBucketB[i + j * WIDTH + bucket * (WIDTH * HEIGHT)] > maxCount)
				{
					maxCount = valueBucketB[i + j * WIDTH + bucket * (WIDTH * HEIGHT)];
					maxBucket = bucket;
					maxTypeA = false;
				}
			}

			finalBucket[i + j * WIDTH].id = maxBucket;
			finalBucket[i + j * WIDTH].isABucket = maxTypeA;
		}
	}

	int *acc = new int[WIDTH * HEIGHT]();
	int *count = new int[WIDTH * HEIGHT]();

	// Average out all the pixel values from the biggest bucket
	for (int frame = 0; frame < FRAMES; frame++)
	{
		string filename = DIRECTORY + "/" + FILESTEM + to_string(1 + frame) + FILETYPE;
		CImg<unsigned char> newImage(filename.c_str());

		cout << "|";

		for (int i = 0; i < WIDTH; i++)
		{
			for (int j = 0; j < HEIGHT; j++)
			{
				int pixel = newImage(i, j, 0, 0);

				if (finalBucket[i + j * WIDTH].isABucket)
				{
					int pixABucket = getABucket(pixel);

					if (pixABucket == finalBucket[i + j * WIDTH].id)
					{
						acc[i + j * WIDTH] += pixel;
						count[i + j * WIDTH]++;
					}
				}
				else
				{
					int pixBBucket = getBBucket(pixel);

					if (pixBBucket == finalBucket[i + j * WIDTH].id)
					{
						acc[i + j * WIDTH] += pixel;
						count[i + j * WIDTH]++;
					}
				}
			}
		}
	}

	// Paint the final result in a window
	for (int i = 0; i < WIDTH; i++)
	{
		for (int j = 0; j < HEIGHT; j++)
		{
			visu(i, j) = acc[i + j * WIDTH] / count[i + j * WIDTH];
		}
		main_disp.render(visu);
		main_disp.paint();

	}

	cout << endl;

	while (!main_disp.is_closed())
	{
		main_disp.wait();
	}


	delete valueBucketA;

	return 0;
}