// ImageProcessor
// Class to handle the processing of the image sequence

#include "imageProcessor.h"

ImageProcessor::ImageProcessor()
{
	width = 480;
	height = 480;

	minVal = 0;
	maxVal = 255;
	bucketSize = 8;
	buckets = (maxVal + 1) / bucketSize;

	fileStem = "picpick";
	fileType = ".png";
}

ImageProcessor::~ImageProcessor()
{
}

void ImageProcessor::addFile(string fileName)
{
	if(!fileName.empty())
		fileNames.push_back(fileName);
}

void ImageProcessor::setFrames(int newFrames)
{
	frames = newFrames;
}

void ImageProcessor::setInputDirectory(string newDir)
{
	inputDirectory = newDir;
}

void ImageProcessor::setBucketSize(int newSize)
{
	bucketSize = 8;
	buckets = (maxVal + 1) / bucketSize;
}

int ImageProcessor::getABucket(int value)
{
	if (value < minVal)
		return 0;

	if (value > maxVal)
		return buckets - 1;

	return value / bucketSize;
}

int ImageProcessor::getBBucket(int value)
{
	return getABucket(value + (bucketSize / 2));
}

void ImageProcessor::processSequence()
{
	CImg<unsigned char> visu(width, height, 1, 3, 0);
	CImgDisplay main_disp(width, height, "Reconstructed background");

	// Buckets store the number of times in the image sequence that the pixel has a value that falls in the given range, or "bucket"
	// A-buckets span the whole range from 0 to 255
	// B-buckets are offset from A-buckets by half of the bucket size and cover a subset of the whole color range
	//
	// The motivation for having two sets of buckets are cases when the pixel values cluster around a bucket boundary and would be split in two.
	// Having an additional set of buckets which are centered around these boundaries helps catch these values and keeps them together.
	vector<int> valueBucketA(width * height * buckets);
	vector<int> valueBucketB(width * height * buckets);
	vector<BucketEntry> finalBucket(width * height);

	cout << endl << "Reading:\t";

	// Read image frames and count the buckets
	for(auto &file : fileNames)
	{
		CImg<unsigned char> newImage(file.c_str());

		std::cout << "|";

		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				int pixel = newImage(i, j, 0, 0);

				int a_bucket = getABucket(pixel);
				valueBucketA[i + j * width + a_bucket * (width * height)]++;

				int b_bucket = getBBucket(pixel);
				valueBucketB[i + j * width + b_bucket * (width * height)]++;

				visu(i, j) = 127;
			}
		}
	}

	// Find the biggest bucket
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int maxCount = 0;
			int maxBucket = -1;
			bool maxTypeA = true;

			for (int bucket = 0; bucket < buckets; bucket++)
			{
				if (valueBucketA[i + j * width + bucket * (width * height)] > maxCount)
				{
					maxCount = valueBucketA[i + j * width + bucket * (width * height)];
					maxBucket = bucket;
					maxTypeA = true;
				}
				if (valueBucketB[i + j * width + bucket * (width * height)] > maxCount)
				{
					maxCount = valueBucketB[i + j * width + bucket * (width * height)];
					maxBucket = bucket;
					maxTypeA = false;
				}
			}

			finalBucket[i + j * width].id = maxBucket;
			finalBucket[i + j * width].isABucket = maxTypeA;
		}
	}

	vector<int> accRed(width * height);
	vector<int> accGreen(width * height);
	vector<int> accBlue(width * height);
	vector<int> count(width * height);

	cout << endl << "Averaging:\t";

	// Average out all the pixel values from the biggest bucket
	for (auto &file : fileNames)
	{
		CImg<unsigned char> newImage(file.c_str());

		cout << "|";

		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				int pixelRed = newImage(i, j, 0, 0);

				if (finalBucket[i + j * width].isABucket)
				{
					int pixABucket = getABucket(pixelRed);

					if (pixABucket == finalBucket[i + j * width].id)
					{
						accRed[i + j * width] += pixelRed;
						accGreen[i + j * width] += newImage(i, j, 0, 1);
						accBlue[i + j * width] += newImage(i, j, 0, 2);
						count[i + j * width]++;
					}
				}
				else
				{
					int pixBBucket = getBBucket(pixelRed);

					if (pixBBucket == finalBucket[i + j * width].id)
					{
						accRed[i + j * width] += pixelRed;
						accGreen[i + j * width] += newImage(i, j, 0, 1);
						accBlue[i + j * width] += newImage(i, j, 0, 2);
						count[i + j * width]++;
					}
				}
			}
		}
	}

	// Paint the final result in a window
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			visu(i, j, 0, 0) = accRed[i + j * width] / count[i + j * width];
			visu(i, j, 0, 1) = accGreen[i + j * width] / count[i + j * width];
			visu(i, j, 0, 2) = accBlue[i + j * width] / count[i + j * width];
		}
		main_disp.render(visu);
		main_disp.paint();

	}

	cout << endl;

	while (!main_disp.is_closed())
	{
		main_disp.wait();
	}
}
