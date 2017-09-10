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

    confLevel = 0.25f;
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

    printImageData();
}

// Print data about the image and the current settings
void ImageProcessor::printImageData()
{
    std::cout << "Image data" << std::endl;
    std::cout << "\tFrames:\t\t" << frames << std::endl;
    std::cout << "\tWidth:\t\t" << width << std::endl;
    std::cout << "\tHeight:\t\t" << height << std::endl;
    std::cout << "\tChannels:\t" << channels << std::endl << std::endl;

    std::cout << "Settings" << std::endl;
    std::cout << "\tBuckets:\t" << buckets << std::endl;
    std::cout << "\tBucket size:\t" << bucketSize << std::endl;
    std::cout << "\tConfidence:\t" << confLevel << std::endl;
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

// Set the confidence level as bucket hits / framecount
void ImageProcessor::setConfidenceLevel(float newConf)
{
    confLevel = newConf;
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

void ImageProcessor::printPixelInformation(int x, int y)
{
    int idx = x + y * width;

    std::cout << std::endl << "Pixel information for " << x << ", " << y << std::endl;

    std::cout << std::endl << "\tA Buckets: ";
    for(int bucket = 0; bucket < buckets; bucket++)
    {
        std::cout << (int) bucketData[0]->bucketA[idx + bucket * size] << " ";
    }

    std::cout << std::endl << "\tB Buckets: ";
    for(int bucket = 0; bucket < buckets; bucket++)
    {
        std::cout << (int) bucketData[0]->bucketB[idx + bucket * size] << " ";
    }

    std::cout << std::endl;
}

// Create final color image and a confidence mask, then display them
void ImageProcessor::createFinal()
{
    int firstPassFail = 0;
    int secondPassFail = 0;

    int confFrames = confLevel * frames;
    confFrames = std::max(confFrames, 1);

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
    std::vector<bool> cleared(size);

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

                int hits = 0;

                for(int channel = 0; channel < channels; channel++)
                {
                    int pixel = newImage(i, j, 0, channel);

                    total[channel][idx] += pixel;

                    BucketEntry entry = bucketData[channel]->finalBucket[idx];

                    if(entry.isABucket && entry.id != getABucket(pixel))
                        continue;

                    if(!entry.isABucket && entry.id != getBBucket(pixel))
                        continue;

                    hits++;
                }

                if(hits == channels)
                {
                    for(int k = 0; k < channels; k++)
                    {
                        acc[k][idx] += newImage(i, j, 0, k);
                    }

                    count[idx]++;
                }
            }
        }
    }

    // Count failed pixels for 1st pass
    for(int i = 0; i < width; i++)
    {
        for(int j = 0; j < height; j++)
        {
            int idx = i + j * width;

            if(count[idx] < confFrames)
            {
                firstPassFail++;
                count[idx] = 0;

                for(int channel = 0; channel < channels; channel++)
                {
                    acc[channel][idx] = 0.0f;
                }
            }
            else
                cleared[idx] = true;
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

                if(cleared[idx])
                    continue;

                int maxDiff = -1;
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

            if(count[idx] < confFrames)
            {
                secondPassFail++;

                confidenceImage(i, j, 0, 0) = 255;
                confidenceImage(i, j, 0, 1) /= 2;
                confidenceImage(i, j, 0, 2) /= 2;

                for(int channel = 0; channel < channels; channel++)
                {
                    float val = (float) total[channel][idx] / frames;
                    reconstructionImage(i, j, 0, channel) = (unsigned char) val;
                }
            }
        }

        aux_disp.render(confidenceImage);
        aux_disp.paint();

        main_disp.render(reconstructionImage);
        main_disp.paint();
    }

    std::cout << std::endl;
    std::cout << std::endl << "Processing finished." << std::endl;
    std::cout << std::endl << "1st pass failed pixels: " << firstPassFail;
    std::cout << std::endl << "2nd pass failed pixels: " << secondPassFail << std::endl;

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

        if(main_disp.button())
        {
            int x = main_disp.mouse_x();
            int y = main_disp.mouse_y();

            if(x >= 0 && x < width && y >= 0 && y < height)
            {
                printPixelInformation(x, y);
            }

        }
    }
}
