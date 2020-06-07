// ImageProcessor
// Class to handle the processing of the image sequence
#include "image_processor.h"

#pragma warning( push )
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#include <iostream>
#include <omp.h>
#include <CImg.h>
#pragma warning( pop )

namespace
{
    const int kDefaultWidth = 480;
    const int kDefaultHeight = 480;
    const float kDefaultConfidenceLevel = 0.2f;
    const int kMaxBrightnessValue = 255;
}

ImageProcessor::ImageProcessor()
{
    _width = kDefaultWidth;
    _height = kDefaultHeight;
    _size = _width * _height;

    _count.resize(_size);
    _cleared.resize(_size);

    _minVal = 0;
    _maxVal = kMaxBrightnessValue;
    _bucketSize = 8;
    _buckets = (_maxVal + 1) / _bucketSize;

    _confLevel = kDefaultConfidenceLevel;
}

ImageProcessor::~ImageProcessor() 
{
}

// Set input file sequence
void ImageProcessor::setFiles(const std::vector<std::string>& fn)
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

    _frames = static_cast<int>(fileNames.size());

    cimg_library::CImg<unsigned char> inspectImage(fileNames[0].c_str());
    _width = inspectImage.width();
    _height = inspectImage.height();
    _size = _width * _height;
    _channels = inspectImage.spectrum();

    _count.resize(_size);
    _cleared.resize(_size);

    printImageData();
}

// Print data about the image and the current settings
void ImageProcessor::printImageData() const
{
    std::cout << "Image data" << std::endl;
    std::cout << "\tFrames:\t\t" << _frames << std::endl;
    std::cout << "\tWidth:\t\t" << _width << std::endl;
    std::cout << "\tHeight:\t\t" << _height << std::endl;
    std::cout << "\tChannels:\t" << _channels << std::endl << std::endl;

    std::cout << "Settings" << std::endl;
    std::cout << "\tBuckets:\t" << _buckets << std::endl;
    std::cout << "\tBucket size:\t" << _bucketSize << std::endl;
    std::cout << "\tConfidence:\t" << _confLevel << std::endl;
}

// Set up the data structure to store bucket information
void ImageProcessor::initializeData()
{
    for (int i = 0; i < _channels; i++)
    {
        bucketData.push_back(BucketData<BucketType>(_width, _height, _buckets));
    }
}

// Set the size of a bucket in terms of color intensity values
void ImageProcessor::setBucketSize(int newSize)
{
    _bucketSize = newSize;
    _buckets = (_maxVal + 1) / _bucketSize;
}

void ImageProcessor::setBitDepth(int newDepth)
{
    _bitDepth = newDepth;
}

// Set the confidence level as bucket hits / framecount
void ImageProcessor::setConfidenceLevel(float newConf) 
{
    _confLevel = newConf;
}

// Find the correspoding A Bucket for the color intensity value
int ImageProcessor::getABucket(int value) const
{
    if (value < _minVal)
    {
        return 0;
    }

    if (value > _maxVal)
    {
        return _buckets - 1;
    }

    return value / _bucketSize;
}

// Find the corresponding B Bucket for the color intensity value
int ImageProcessor::getBBucket(int value) const
{
    value += _bucketSize / 2;

    if (value < _minVal)
    {
        return 0;
    }

    if (value > _maxVal - _bucketSize / 2)
    {
        return _buckets - 1;
    }

    return value / _bucketSize;
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
    for (auto& file : fileNames) 
    {
        cimg_library::CImg<unsigned char> newImage(file.c_str());

        std::cout << "|" << std::flush;

#pragma omp parallel for
        for (int i = 0; i < _width; i++) 
        {
            for (int j = 0; j < _height; j++) 
            {
                for (int channel = 0; channel < _channels; channel++) 
                {
                    int pixel = newImage(i, j, 0, channel);

                    int a_bucket = getABucket(pixel);
                    bucketData[channel].bucketA[i + j * _width + a_bucket * _size]++;

                    int b_bucket = getBBucket(pixel);
                    bucketData[channel].bucketB[i + j * _width + b_bucket * _size]++;
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

#pragma omp parallel for
    // Find the biggest bucket
    for (int i = 0; i < _width; i++) 
    {
        for (int j = 0; j < _height; j++) 
        {
            int idx = i + j * _width;

            for (int channel = 0; channel < _channels; channel++) 
            {
                int maxCount = 0;
                BucketType maxBucket = 0;
                bool maxTypeA = true;

                for (int bucket = 0; bucket < _buckets; bucket++) 
                {
                    if (bucketData[channel].bucketA[i + j * _width + bucket * _size] > maxCount) 
                    {
                        maxCount = bucketData[channel].bucketA[i + j * _width + bucket * _size];
                        maxBucket = static_cast<BucketType>(bucket);
                        maxTypeA = true;
                    }
                    if (bucketData[channel].bucketB[i + j * _width + bucket * _size] > maxCount) 
                    {
                        maxCount = bucketData[channel].bucketB[i + j * _width + bucket * _size];
                        maxBucket = static_cast<BucketType>(bucket);
                        maxTypeA = false;
                    }
                }

                bucketData[channel].finalBucket[idx].id = maxBucket;
                bucketData[channel].finalBucket[idx].isABucket = maxTypeA;
                bucketData[channel].finalBucket[idx].diff = maxCount;
            }
        }
    }
}

void ImageProcessor::printPixelInformation(int x, int y) const
{
    int idx = x + y * _width;

    std::cout << std::endl << "Pixel information for " << x << ", " << y << std::endl;

    std::cout << std::endl << "\tA Buckets: ";
    for (int bucket = 0; bucket < _buckets; bucket++)
    {
        std::cout << static_cast<int>(bucketData[0].bucketA[idx + bucket * _size]) << " ";
    }

    std::cout << std::endl << "\tB Buckets: ";
    for (int bucket = 0; bucket < _buckets; bucket++)
    {
        std::cout << static_cast<int>(bucketData[0].bucketB[idx + bucket * _size]) << " ";
    }

    std::cout << std::endl;
}

void ImageProcessor::firstPass(/*vec2d& acc, vec2d& total, std::vector<int>& count*/)
{
    std::cout << std::endl << "1st pass:\t";

    for (auto& file : fileNames)
    {
        cimg_library::CImg<unsigned char> newImage(file.c_str());

        std::cout << "|" << std::flush;

#pragma omp parallel for
        for (int i = 0; i < _width; i++) 
        {
            for (int j = 0; j < _height; j++) 
            {
                int idx = i + j * _width;
                int hits = 0;

                for (int channel = 0; channel < _channels; channel++) 
                {
                    int pixel = newImage(i, j, 0, channel);

                    _total[channel][idx] += pixel;

                    BucketEntry<unsigned char> entry = bucketData[channel].finalBucket[idx];

                    if (entry.isABucket && entry.id != getABucket(pixel))
                    {
                        continue;
                    }

                    if (!entry.isABucket && entry.id != getBBucket(pixel))
                    {
                        continue;
                    }

                    hits++;
                }

                if (hits == _channels) 
                {
                    for (int k = 0; k < _channels; k++) 
                    {
                        _acc[k][idx] += newImage(i, j, 0, k);
                    }

                    _count[idx]++;
                }
            }
        }
    }
}

void ImageProcessor::countFailed(int confFrames, int& failed)
{
#pragma omp parallel for
    for (int i = 0; i < _width; i++) 
    {
        for (int j = 0; j < _height; j++) 
        {
            int idx = i + j * _width;

            if (_count[idx] < confFrames) 
            {
                failed++;
                _count[idx] = 0;

                for (int channel = 0; channel < _channels; channel++) 
                {
                    _acc[channel][idx] = 0.0f;
                }
            }
            else
            {
                _cleared[idx] = true;
            }
        }
    }
}

void ImageProcessor::secondPass()
{
    std::cout << std::endl << "2nd pass:\t";

    for (auto& file : fileNames)
    {
        cimg_library::CImg<unsigned char> newImage(file.c_str());

        std::cout << "|" << std::flush;

#pragma omp parallel for
        for (int i = 0; i < _width; i++) 
        {
            for (int j = 0; j < _height; j++) 
            {
                int idx = i + j * _width;

                if (_cleared[idx])
                {
                    continue;
                }

                int maxDiff = -1;
                int maxChannel = -1;

                std::vector<BucketEntry<unsigned char>> entry(_channels);
                std::vector<int> pixel(_channels);

                for (int channel = 0; channel < _channels; channel++) 
                {
                    entry[channel] = bucketData[channel].finalBucket[idx];
                    pixel[channel] = newImage(i, j, 0, channel);

                    if (entry[channel].diff > maxDiff) 
                    {
                        maxDiff = entry[channel].diff;
                        maxChannel = channel;
                    }
                }

                BucketEntry<unsigned char> maxEntry = entry[maxChannel];

                if (maxEntry.isABucket && maxEntry.id != getABucket(pixel[maxChannel]))
                {
                    continue;
                }

                if (!maxEntry.isABucket && maxEntry.id != getBBucket(pixel[maxChannel]))
                {
                    continue;
                }

                for (int channel = 0; channel < _channels; channel++) 
                {
                    _acc[channel][idx] += pixel[channel];
                }

                _count[idx]++;
            }
        }
    }
}

void ImageProcessor::drawImages(int confFrames, int& firstPassFail, int& secondPassFail)
{
    // Paint the final result in a window
    cimg_library::CImg<unsigned char> reconstructionImage(_width, _height, 1, 3, 0);
    cimg_library::CImgDisplay mainDisp(_width, _height, "Reconstructed background");

    // Paint the confidence mask
    cimg_library::CImg<unsigned char> confidenceImage(_width, _height, 1, 3, 0);
    cimg_library::CImgDisplay auxDisp(_width, _height, "Confidence mask");

    for (int i = 0; i < _width; i++) 
    {
#pragma omp parallel for
        for (int j = 0; j < _height; j++) 
        {
            int idx = i + j * _width;

            reconstructionImage(i, j, 0, 0) = static_cast<unsigned char>(_acc[0][idx] / _count[idx]);
            reconstructionImage(i, j, 0, 1) = static_cast<unsigned char>(_acc[1][idx] / _count[idx]);
            reconstructionImage(i, j, 0, 2) = static_cast<unsigned char>(_acc[2][idx] / _count[idx]);

            int confidence = static_cast<int>(_count[i + j * _width] * (256.0f / _frames));
            confidence = std::min(confidence, 255);

            confidenceImage(i, j, 0, 0) = static_cast<unsigned char>(confidence);
            confidenceImage(i, j, 0, 1) = static_cast<unsigned char>(confidence);
            confidenceImage(i, j, 0, 2) = static_cast<unsigned char>(confidence);

            if (_count[idx] < confFrames) 
            {
                secondPassFail++;

                confidenceImage(i, j, 0, 0) = 255;
                confidenceImage(i, j, 0, 1) /= 2;
                confidenceImage(i, j, 0, 2) /= 2;

                for (int channel = 0; channel < _channels; channel++) 
                {
                    float val = static_cast<float>(_total[channel][idx]) / _frames;
                    reconstructionImage(i, j, 0, channel) = static_cast<unsigned char>(val);
                }
            }
        }

        auxDisp.render(confidenceImage);
        auxDisp.paint();

        mainDisp.render(reconstructionImage);
        mainDisp.paint();
    }

    // Write the final color image to file
    std::remove("output.png");
    reconstructionImage.save_png("output.png");

    // Write out the confidence mask
    std::remove("confidence.png");
    confidenceImage.save_png("confidence.png");

    std::cout << std::endl;
    std::cout << std::endl << "Processing finished." << std::endl;
    std::cout << std::endl << "1st pass failed pixels: " << firstPassFail;
    std::cout << std::endl << "2nd pass failed pixels: " << secondPassFail << std::endl;

    std::cout << std::endl;

    while (!mainDisp.is_closed()) 
    {
        mainDisp.wait();

        if (mainDisp.button()) 
        {
            int x = mainDisp.mouse_x();
            int y = mainDisp.mouse_y();

            if (x >= 0 && x < _width && y >= 0 && y < _height) 
            {
                printPixelInformation(x, y);
            }
        }
    }
}

// Create final color image and a confidence mask, then display them
void ImageProcessor::createFinal()
{
    int firstPassFail = 0;
    int secondPassFail = 0;

    int confFrames = static_cast<int>(std::floorf(_confLevel * _frames));
    confFrames = std::max(confFrames, 1);

    //std::vector<std::vector<float>> acc;
    //std::vector<std::vector<float>> total;

    for (int channel = 0; channel < _channels; channel++) 
    {
        std::vector<float> tempAcc(_size);
        _acc.push_back(tempAcc);

        std::vector<float> tempTotal(_size);
        _total.push_back(tempTotal);
    }

    firstPass(/*acc, total, count*/);
    countFailed(/*acc, count, cleared, */confFrames, firstPassFail);
    secondPass(/*acc, count, cleared*/);
    drawImages(/*acc, total, count, */confFrames, firstPassFail, secondPassFail);
}
