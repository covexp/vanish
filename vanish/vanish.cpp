// Vanish
// Remove transient objects from an image sequence

#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include "../../cimg/CImg.h"

namespace opt = boost::program_options;

using namespace cimg_library;
using namespace std;

const int MINVAL = 0;
const int MAXVAL = 255;
const int WIDTH = 480;
const int HEIGHT = 480;
const int FRAMES = 67;
const int BUCKETSIZE = 8;
const int BUCKETS = (MAXVAL + 1) / BUCKETSIZE;

const string DEFAULT_DIRECTORY = "input";
const string FILESTEM = "picpick";
const string FILETYPE = ".png";

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

void processSequence(string inputDirectory)
{
	CImg<unsigned char> visu(WIDTH, HEIGHT, 1, 3, 0);
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
		char pad[256];
		sprintf_s(pad, "%03d", 1 + frame);
		string padString(pad);

		string filename = inputDirectory + "/" + FILESTEM + padString + FILETYPE;

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

	int *accRed = new int[WIDTH * HEIGHT]();
	int *accGreen = new int[WIDTH * HEIGHT]();
	int *accBlue = new int[WIDTH * HEIGHT]();
	int *count = new int[WIDTH * HEIGHT]();

	// Average out all the pixel values from the biggest bucket
	for (int frame = 0; frame < FRAMES; frame++)
	{
		char pad[256];
		sprintf_s(pad, "%03d", 1 + frame);
		string padString(pad);

		string filename = inputDirectory + "/" + FILESTEM + padString + FILETYPE;
		CImg<unsigned char> newImage(filename.c_str());

		cout << "|";

		for (int i = 0; i < WIDTH; i++)
		{
			for (int j = 0; j < HEIGHT; j++)
			{
				int pixelRed = newImage(i, j, 0, 0);

				if (finalBucket[i + j * WIDTH].isABucket)
				{
					int pixABucket = getABucket(pixelRed);

					if (pixABucket == finalBucket[i + j * WIDTH].id)
					{
						accRed[i + j * WIDTH] += pixelRed;
						accGreen[i + j * WIDTH] += newImage(i, j, 0, 1);
						accBlue[i + j * WIDTH] += newImage(i, j, 0, 2);
						count[i + j * WIDTH]++;
					}
				}
				else
				{
					int pixBBucket = getBBucket(pixelRed);

					if (pixBBucket == finalBucket[i + j * WIDTH].id)
					{
						accRed[i + j * WIDTH] += pixelRed;
						accGreen[i + j * WIDTH] += newImage(i, j, 0, 1);
						accBlue[i + j * WIDTH] += newImage(i, j, 0, 2);
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
			visu(i, j, 0, 0) = accRed[i + j * WIDTH] / count[i + j * WIDTH];
			visu(i, j, 0, 1) = accGreen[i + j * WIDTH] / count[i + j * WIDTH];
			visu(i, j, 0, 2) = accBlue[i + j * WIDTH] / count[i + j * WIDTH];
		}
		main_disp.render(visu);
		main_disp.paint();

	}

	cout << endl;

	while (!main_disp.is_closed())
	{
		main_disp.wait();
	}

	// Free up memory from arrays
	delete[] valueBucketA;
	delete[] valueBucketB;
	delete[] finalBucket;
	delete[] accRed;
	delete[] accGreen;
	delete[] accBlue;
	delete[] count;
}

int main(int argc, char *argv[])
{
	// Welcome message
	cout << "Vanish - Version 0.02" << endl;

	// Command line options with boost::program_options
	opt::options_description desc("Allowed options");
	desc.add_options()
		("help", "show help message")
		("dir", opt::value<string>(), "directory of input image sequence")
		("frames", opt::value<int>(), "number of frames")
		("bucket", opt::value<int>(), "bucket size")
		("w", opt::value<int>(), "image width")
		("h", opt::value<int>(), "image height")
		;

	opt::variables_map vm;
	opt::store(opt::parse_command_line(argc, argv, desc), vm);
	opt::notify(vm);

	if (vm.count("help"))
	{
		cout << desc << endl;
		return 1;
	}

	string inputDirectory;

	if (vm.count("dir"))
	{
		cout << "Input directory was: " << vm["dir"].as<string>() << endl;
		inputDirectory = vm["dir"].as<string>();
	}
	else
	{
		cout << "Directory was not set." << endl;
		inputDirectory = DEFAULT_DIRECTORY;
	}

	// Process the specified image sequence
	processSequence(inputDirectory);

	return 0;
}