// Vanish
// Remove transient objects from an image sequence

#include <iostream>
#include <string>
#include "../../cimg/CImg.h"

using namespace cimg_library;
using namespace std;

#define WIDTH 480
#define HEIGHT 480
#define FRAMES 10
#define BUCKETSIZE 32
#define BUCKETS 256 / BUCKETSIZE

string DIRECTORY = "logi";
string FILESTEM = "salsa (";
string FILETYPE = ").jpg";

int main(void)
{
	cout << "Vanishing... " << endl;

	CImg<unsigned char> visu(WIDTH, HEIGHT, 1, 1, 0);
	CImgDisplay main_disp(WIDTH, HEIGHT, "Everything must go");

	int *buckets = new int[WIDTH * HEIGHT * BUCKETS]();
	int *finalBucket = new int[WIDTH * HEIGHT]();

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
				int bucket = pixel / BUCKETSIZE;
				buckets[i + j * WIDTH + bucket * (WIDTH * HEIGHT)]++;

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

			for (int bucket = 0; bucket < BUCKETS; bucket++)
			{
				if (buckets[i + j * WIDTH + bucket * (WIDTH * HEIGHT)] > maxCount)
				{
					maxCount = buckets[i + j * WIDTH + bucket * (WIDTH * HEIGHT)];
					maxBucket = bucket;
				}
			}

			finalBucket[i + j * WIDTH] = maxBucket;
		}
	}

	int *acc = new int[WIDTH * HEIGHT]();
	int *count = new int[WIDTH * HEIGHT]();

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

				if (pixel > (finalBucket[i + j * WIDTH] - 1) * BUCKETSIZE && pixel < (finalBucket[i + j * WIDTH] + 1) * BUCKETSIZE)
				{
					acc[i + j * WIDTH] += pixel;
					count[i + j * WIDTH]++;
				}
			}
		}
	}

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


	delete buckets;

	return 0;
}