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
	if (bucketData)
		delete bucketData;
}

void ImageProcessor::setFiles(std::vector<std::string> fn)
{
	fileNames = fn;

	inferParameters();
	initializeData();
}

void ImageProcessor::inferParameters()
{
	if (fileNames.size() <= 0)
	{
		std::cerr << "Image list empty. Exiting." << std::endl;
		exit(EXIT_FAILURE);
	}

	frames = fileNames.size();

	cimg::CImg<unsigned char> inspectImage(fileNames[0].c_str());
	width = inspectImage.width();
	height = inspectImage.height();
	channels = inspectImage.spectrum();

	std::cout << "Image data" << std::endl;
	std::cout << "\tFrames:\t\t" << frames << std::endl;
	std::cout << "\tWidth:\t\t" << width << std::endl;
	std::cout << "\tHeight:\t\t" << height << std::endl;
	std::cout << "\tChannels:\t" << channels << std::endl;
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
	countBuckets();
	findBiggestBucket();
	refineSolution();
	createOutput();
}

void ImageProcessor::countBuckets()
{
	std::cout << std::endl << "Reading:\t";

	// Read image frames and count the buckets
	for (auto &file : fileNames)
	{
		cimg::CImg<unsigned char> newImage(file.c_str());

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
			}
		}
	}
}

void ImageProcessor::findBiggestBucket()
{
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
			bucketData->finalBucket[i + j * width].diff = maxCount;
		}
	}
}

void ImageProcessor::refineSolution()
{
	short int confidenceLow = (short int)(0.5f * frames);
	short int confidenceHigh = (short int)(0.75f * frames);

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int curIndex = i + j * width;

			// Check to see if the pixel has a low-confidence solution
			if (bucketData->finalBucket[curIndex].diff < confidenceLow)
			{
				bool done = false;

				// Iterate through the pixel neighborhood for high confidence values
				for (int m = i - 1; m < i + 1 && !done; m++)
				{
					for (int n = j - 1; n < j + 1 && !done; n++)
					{
						if (m >= 0 && m < width && n >= 0 && n < height)
						{
							int neighIndex = m + n * width;

							if (bucketData->finalBucket[neighIndex].diff > confidenceHigh)
							{
								// Replace the current pixel's bucket with the neighbor's
								bucketData->finalBucket[curIndex] = bucketData->finalBucket[neighIndex];
								// Zero out the confidence
								bucketData->finalBucket[curIndex].diff = 0;

								// Break out of the two innermost loops
								done = true;
							}
						}
					}
				}
			}
		}
	}
}

void ImageProcessor::createOutput()
{
	std::vector<float> accRed(width * height);
	std::vector<float> accGreen(width * height);
	std::vector<float> accBlue(width * height);
	std::vector<int> count(width * height);

	std::cout << std::endl << "Averaging:\t";

	// Average out all the pixel values from the biggest bucket
	for (auto &file : fileNames)
	{
		cimg::CImg<unsigned char> newImage(file.c_str());

		std::cout << "|";

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
	cimg::CImg<unsigned char> reconstructionImage(width, height, 1, 3, 0);
	cimg::CImgDisplay main_disp(width, height, "Reconstructed background");

	cimg::CImg<unsigned char> confidenceImage(width, height, 1, 3, 0);
	cimg::CImgDisplay aux_disp(width, height, "Confidence mask");

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			reconstructionImage(i, j, 0, 0) = (unsigned char) (accRed[i + j * width] / count[i + j * width]);
			reconstructionImage(i, j, 0, 1) = (unsigned char) (accGreen[i + j * width] / count[i + j * width]);
			reconstructionImage(i, j, 0, 2) = (unsigned char) (accBlue[i + j * width] / count[i + j * width]);

			// Paint a red pixel where the confidence is zero
			if(bucketData->finalBucket[i + j * width].diff == 0)
				confidenceImage(i, j, 0, 0) = maxVal;
			else
			{
				confidenceImage(i, j, 0, 0) = (unsigned char)(bucketData->finalBucket[i + j * width].diff * maxVal / frames);
				confidenceImage(i, j, 0, 1) = (unsigned char)(bucketData->finalBucket[i + j * width].diff * maxVal / frames);
				confidenceImage(i, j, 0, 2) = (unsigned char)(bucketData->finalBucket[i + j * width].diff * maxVal / frames);
			}
		}
		main_disp.render(reconstructionImage);
		main_disp.paint();

		aux_disp.render(confidenceImage);
		aux_disp.paint();
	}

	std::cout << std::endl;

	while (!main_disp.is_closed())
	{
		main_disp.wait();
	}
}
