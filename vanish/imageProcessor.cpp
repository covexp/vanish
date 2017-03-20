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
}

ImageProcessor::~ImageProcessor()
{
}

void ImageProcessor::setFiles(vector<string> fn)
{
	fileNames = fn;

	inferParameters();
	initializeData();
}

void ImageProcessor::inferParameters()
{
	if (fileNames.size() <= 0)
	{
		cerr << "Image list empty. Exiting." << endl;
		exit(EXIT_FAILURE);
	}

	frames = fileNames.size();

	CImg<unsigned char> inspectImage(fileNames[0].c_str());
	width = inspectImage.width();
	height = inspectImage.height();
	channels = inspectImage.depth();
}

void ImageProcessor::initializeData()
{
	bucketData = new BucketData(width, height, buckets);
}

void ImageProcessor::setBucketSize(int newSize)
{
	bucketSize = newSize;
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
				bucketData->valueBucketA[i + j * width + a_bucket * (width * height)]++;

				int b_bucket = getBBucket(pixel);
				bucketData->valueBucketB[i + j * width + b_bucket * (width * height)]++;

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
				if (bucketData->valueBucketA[i + j * width + bucket * (width * height)] > maxCount)
				{
					maxCount = bucketData->valueBucketA[i + j * width + bucket * (width * height)];
					maxBucket = bucket;
					maxTypeA = true;
				}
				if (bucketData->valueBucketB[i + j * width + bucket * (width * height)] > maxCount)
				{
					maxCount = bucketData->valueBucketB[i + j * width + bucket * (width * height)];
					maxBucket = bucket;
					maxTypeA = false;
				}
			}

			bucketData->finalBucket[i + j * width].id = maxBucket;
			bucketData->finalBucket[i + j * width].isABucket = maxTypeA;
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

				if (bucketData->finalBucket[i + j * width].isABucket)
				{
					int pixABucket = getABucket(pixelRed);

					if (pixABucket == bucketData->finalBucket[i + j * width].id)
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

					if (pixBBucket == bucketData->finalBucket[i + j * width].id)
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
