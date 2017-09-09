// ImageProcessor
// Class to handle the processing of the image sequence

#include "imageProcessor.h"

ImageProcessor::ImageProcessor()
{
    width = 480;
    height = 480;
    size = width * height;

    minVal = 0;
    maxVal = 255;
    bucketSize = 8;
    buckets = (maxVal + 1) / bucketSize;
}

ImageProcessor::~ImageProcessor()
{
    if(bucketData.size() > 0)
    {
        for(auto data : bucketData)
            delete data;
    }
}

// Set input file sequence
void ImageProcessor::setFiles(std::vector<std::string> fn)
{
    fileNames = fn;

    inferParameters();
    initializeData();
}

// Infer processor parameters from the first file
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
    size = width * height;
    channels = inspectImage.spectrum();

    std::cout << "Image data" << std::endl;
    std::cout << "\tFrames:\t\t" << frames << std::endl;
    std::cout << "\tWidth:\t\t" << width << std::endl;
    std::cout << "\tHeight:\t\t" << height << std::endl;
    std::cout << "\tChannels:\t" << channels << std::endl;
    std::cout << "\tBuckets:\t" << buckets << std::endl;
    std::cout << "\tBucket size:\t" << bucketSize << std::endl;
}

// Set up the data structure to store bucket information
void ImageProcessor::initializeData()
{
    for(int i = 0; i < channels; i++)
        bucketData.push_back(new BucketData(width, height, buckets));
}

// Set the size of a bucket in terms of color intensity values
void ImageProcessor::setBucketSize(int newSize)
{
    bucketSize = newSize;
    buckets = (maxVal + 1) / bucketSize;
}

// Find the correspoding A Bucket for the color intensity value
int ImageProcessor::getABucket(int value)
{
    if (value < minVal)
        return 0;

    if (value > maxVal)
        return buckets - 1;

    return value / bucketSize;
}

// Find the corresponding B Bucket for the color intensity value
int ImageProcessor::getBBucket(int value)
{
    value += bucketSize / 2;

    if (value < minVal)
        return 0;

    if (value > maxVal - bucketSize / 2)
        return buckets - 1;

    return value / bucketSize;
}

// Process the image sequence and create final output
void ImageProcessor::processSequence()
{
    countBuckets();
    findBiggestBucket();
    createFinal();
}

// Read the image files and count the pixel values into buckets
void ImageProcessor::countBuckets()
{
    std::cout << std::endl << "Reading:\t";

    // Read image frames and count the buckets
    for (auto &file : fileNames)
    {
        cimg::CImg<unsigned char> newImage(file.c_str());

        std::cout << "|" << std::flush;

        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                for(int channel = 0; channel < channels; channel++)
                {
                    int pixel = newImage(i, j, 0, channel);

                    int a_bucket = getABucket(pixel);
                    bucketData[channel]->bucketA[i + j * width + a_bucket * size]++;

                    int b_bucket = getBBucket(pixel);
                    bucketData[channel]->bucketB[i + j * width + b_bucket * size]++;
                }
            }
        }
    }

    std::cout << std::endl << "Finished reading files...";
}

// Find the biggest bucket for each pixel
void ImageProcessor::findBiggestBucket()
{
    std::cout << std::endl << "Finding the biggest bucket..." << std::flush;

    // Find the biggest bucket
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int idx = i + j * width;

            for (int channel = 0; channel < channels; channel++)
            {
                int maxCount = 0;
                int maxBucket = -1;
                bool maxTypeA = true;

                for(int bucket = 0; bucket < buckets; bucket++)
                {
                    if(bucketData[channel]->bucketA[i + j * width + bucket * size] > maxCount)
                    {
                        maxCount = bucketData[channel]->bucketA[i + j * width + bucket * size];
                        maxBucket = bucket;
                        maxTypeA = true;
                    }
                    if(bucketData[channel]->bucketB[i + j * width + bucket * size] > maxCount)
                    {
                        maxCount = bucketData[channel]->bucketB[i + j * width + bucket * size];
                        maxBucket = bucket;
                        maxTypeA = false;
                    }
                }

                bucketData[channel]->finalBucket[idx].id = maxBucket;
                bucketData[channel]->finalBucket[idx].isABucket = maxTypeA;
                bucketData[channel]->finalBucket[idx].diff = maxCount;
            }
        }
    }
}

// Create final color image and a confidence mask, then display them
void ImageProcessor::createFinal()
{
    int confLevel = frames / 3;

    std::vector<std::vector<float>> acc;
    std::vector<std::vector<float>> total;

    for(int channel = 0; channel < channels; channel++)
    {
        std::vector<float> tempAcc(size);
        acc.push_back(tempAcc);

        std::vector<float> tempTotal(size);
        total.push_back(tempTotal);
    }

    std::vector<int> count(size);

    std::cout << std::endl << "1st pass:\t";

    for (auto &file : fileNames)
    {
        cimg::CImg<unsigned char> newImage(file.c_str());

        std::cout << "|" << std::flush;

        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                int idx = i + j * width;

                for(int channel = 0; channel < channels; channel++)
                {
                    int pixel = newImage(i, j, 0, channel);

                    total[channel][idx] += pixel;

                    BucketEntry entry = bucketData[channel]->finalBucket[idx];

                    if(entry.isABucket && entry.id != getABucket(pixel))
                        continue;

                    if(!entry.isABucket && entry.id != getBBucket(pixel))
                        continue;

                    for(int k = 0; k < channels; k++)
                    {
                        acc[k][idx] += newImage(i, j, 0, k);
                    }

                    count[idx]++;
                }
            }
        }
    }

    std::cout << std::endl << "2nd pass:\t";

    for (auto &file : fileNames)
    {
        cimg::CImg<unsigned char> newImage(file.c_str());

        std::cout << "|" << std::flush;

        for(int i = 0; i < width; i++)
        {
            for(int j = 0; j < height; j++)
            {
                int idx = i + j * width;

                if(count[idx] >= confLevel)
                    continue;

                int maxDiff = 0;
                int maxChannel = -1;

                std::vector<BucketEntry> entry(channels);
                std::vector<int> pixel(channels);

                for(int channel = 0; channel < channels; channel++)
                {
                    entry[channel] = bucketData[channel]->finalBucket[idx];
                    pixel[channel] = newImage(i, j, 0, channel);

                    if(entry[channel].diff > maxDiff)
                    {
                        maxDiff = entry[channel].diff;
                        maxChannel = channel;
                    }
                }

                BucketEntry maxEntry = entry[maxChannel];

                if(maxEntry.isABucket && maxEntry.id != getABucket(pixel[maxChannel]))
                    continue;

                if(!maxEntry.isABucket && maxEntry.id != getBBucket(pixel[maxChannel]))
                    continue;

                for(int channel = 0; channel < channels; channel++)
                {
                    acc[channel][idx] += pixel[channel];
                }

                count[idx]++;
            }
        }
    }

    // Paint the final result in a window
    cimg::CImg<unsigned char> reconstructionImage(width, height, 1, 3, 0);
    cimg::CImgDisplay main_disp(width, height, "Reconstructed background");

    // Paint the confidence mask
    cimg::CImg<unsigned char> confidenceImage(width, height, 1, 3, 0);
    cimg::CImgDisplay aux_disp(width, height, "Confidence mask");

    // Paint the green buckets for debugging
    cimg::CImg<unsigned char> blueBucketImage(width, height, 1, 1, 0);
    cimg::CImgDisplay green_disp(width, height, "Blue buckets");

    int lowConfCount = 0;

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int idx = i + j * width;

            reconstructionImage(i, j, 0, 0) = (unsigned char) (acc[0][idx] / count[idx]);
            reconstructionImage(i, j, 0, 1) = (unsigned char) (acc[1][idx] / count[idx]);
            reconstructionImage(i, j, 0, 2) = (unsigned char) (acc[2][idx] / count[idx]);

            int confidence = (int) (count[i + j * width] * ((float)256.0f / frames));
            confidence = std::min(confidence, 255);

            confidenceImage(i, j, 0, 0) = (unsigned char) confidence;
            confidenceImage(i, j, 0, 1) = (unsigned char) confidence;
            confidenceImage(i, j, 0, 2) = (unsigned char) confidence;

            if(count[idx] < confLevel)
            {
                lowConfCount++;

                confidenceImage(i, j, 0, 0) = 255;
                confidenceImage(i, j, 0, 1) /= 2;
                confidenceImage(i, j, 0, 2) /= 2;

//                printInformation(i, j);

                float newRed = (float) total[0][idx] / frames;
                float newGreen = (float) total[1][idx] / frames;
                float newBlue = (float) total[2][idx] / frames;

//                newRed = 255.0f;
//                newGreen = 255.0f;
//                newBlue = 0.0f;

                reconstructionImage(i, j, 0, 0) = (unsigned char) newRed;
                reconstructionImage(i, j, 0, 1) = (unsigned char) newGreen;
                reconstructionImage(i, j, 0, 2) = (unsigned char) newBlue;
            }

            BucketEntry blueEntry = bucketData[2]->finalBucket[i + j * width];
            unsigned char valueBlue = blueEntry.id * bucketSize;

            if(!blueEntry.isABucket)
            {
                if(valueBlue > bucketSize / 2)
                    valueBlue -= bucketSize / 2;
                else
                    valueBlue = 0;
            }

            blueBucketImage(i, j, 0, 0) = valueBlue;
        }

        main_disp.render(reconstructionImage);
        main_disp.paint();

        aux_disp.render(confidenceImage);
        aux_disp.paint();

        green_disp.render(blueBucketImage);
        green_disp.paint();
    }

    std::cout << std::endl;
    std::cout << std::endl << "Processing finished.";
    std::cout << std::endl << "Low confidence pixels: " << lowConfCount << std::endl;

    std::cout << std::endl;

    // Write the final color image to file
    std::remove("output.png");
    reconstructionImage.save_png("output.png");

    // Write out the confidence mask
    std::remove("confidence.png");
    confidenceImage.save_png("confidence.png");

    while (!main_disp.is_closed())
    {
        main_disp.wait();
    }
}
