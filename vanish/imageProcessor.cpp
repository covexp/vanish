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
    channels = inspectImage.spectrum();

    std::cout << "Image data" << std::endl;
    std::cout << "\tFrames:\t\t" << frames << std::endl;
    std::cout << "\tWidth:\t\t" << width << std::endl;
    std::cout << "\tHeight:\t\t" << height << std::endl;
    std::cout << "\tChannels:\t" << channels << std::endl;
}

// Set up the data structure to store bucket information
void ImageProcessor::initializeData()
{
    bucketData = new BucketData(width, height, buckets);
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
//    refineSolution();
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

        std::cout << "|";

        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                int redPixel = newImage(i, j, 0, 0);
                int greenPixel = newImage(i, j, 0, 1);
                int bluePixel = newImage(i, j, 0, 2);

                int a_bucket = getABucket(redPixel);
                bucketData->redBucketA[i + j * width + a_bucket * (width * height)]++;

                int b_bucket = getBBucket(redPixel);
                bucketData->redBucketB[i + j * width + b_bucket * (width * height)]++;

                a_bucket = getABucket(greenPixel);
                bucketData->greenBucketA[i + j * width + a_bucket * (width * height)]++;

                b_bucket = getBBucket(greenPixel);
                bucketData->greenBucketB[i + j * width + b_bucket * (width * height)]++;

                a_bucket = getABucket(bluePixel);
                bucketData->blueBucketA[i + j * width + a_bucket * (width * height)]++;

                b_bucket = getBBucket(bluePixel);
                bucketData->blueBucketB[i + j * width + b_bucket * (width * height)]++;
            }
        }
    }
}

// Find the biggest bucket for each pixel
void ImageProcessor::findBiggestBucket()
{
    // Find the biggest bucket
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int idx = i + j * width;

            // Red channel
            {
                int redMaxCount = 0;
                int redMaxBucket = -1;
                bool redMaxTypeA = true;

                for (int bucket = 0; bucket < buckets; bucket++)
                {
                    if (bucketData->redBucketA[i + j * width + bucket * (width * height)] > redMaxCount)
                    {
                        redMaxCount = bucketData->redBucketA[i + j * width + bucket * (width * height)];
                        redMaxBucket = bucket;
                        redMaxTypeA = true;
                    }
                    if (bucketData->redBucketB[i + j * width + bucket * (width * height)] > redMaxCount)
                    {
                        redMaxCount = bucketData->redBucketB[i + j * width + bucket * (width * height)];
                        redMaxBucket = bucket;
                        redMaxTypeA = false;
                    }
                }

                bucketData->redFinalBucket[idx].id = redMaxBucket;
                bucketData->redFinalBucket[idx].isABucket = redMaxTypeA;
                bucketData->redFinalBucket[idx].diff = redMaxCount;
            }

            // Green channel
            {
                int greenMaxCount = 0;
                int greenMaxBucket = -1;
                bool greenMaxTypeA = true;

                for (int bucket = 0; bucket < buckets; bucket++)
                {
                    if (bucketData->greenBucketA[i + j * width + bucket * (width * height)] > greenMaxCount)
                    {
                        greenMaxCount = bucketData->greenBucketA[i + j * width + bucket * (width * height)];
                        greenMaxBucket = bucket;
                        greenMaxTypeA = true;
                    }
                    if (bucketData->greenBucketB[i + j * width + bucket * (width * height)] > greenMaxCount)
                    {
                        greenMaxCount = bucketData->greenBucketB[i + j * width + bucket * (width * height)];
                        greenMaxBucket = bucket;
                        greenMaxTypeA = false;
                    }
                }

                bucketData->greenFinalBucket[idx].id = greenMaxBucket;
                bucketData->greenFinalBucket[idx].isABucket = greenMaxTypeA;
                bucketData->greenFinalBucket[idx].diff = greenMaxCount;
            }

            // Blue channel
            {
                int blueMaxCount = 0;
                int blueMaxBucket = -1;
                bool blueMaxTypeA = true;

                for (int bucket = 0; bucket < buckets; bucket++)
                {
                    if (bucketData->blueBucketA[i + j * width + bucket * (width * height)] > blueMaxCount)
                    {
                        blueMaxCount = bucketData->blueBucketA[i + j * width + bucket * (width * height)];
                        blueMaxBucket = bucket;
                        blueMaxTypeA = true;
                    }
                    if (bucketData->blueBucketB[i + j * width + bucket * (width * height)] > blueMaxCount)
                    {
                        blueMaxCount = bucketData->blueBucketB[i + j * width + bucket * (width * height)];
                        blueMaxBucket = bucket;
                        blueMaxTypeA = false;
                    }
                }

                bucketData->blueFinalBucket[idx].id = blueMaxBucket;
                bucketData->blueFinalBucket[idx].isABucket = blueMaxTypeA;
                bucketData->blueFinalBucket[idx].diff = blueMaxCount;
            }
        }
    }
}

// Optional step to let neighboring buckets with stronger confidence influence the bucket values of
// weak confidence pixels
void ImageProcessor::refineSolution()
{
    // Removed pending a better implementation

}

// Create final color image and a confidence mask, then display them
void ImageProcessor::createFinal()
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
                int idx = i + j * width;

                int pixelRed = newImage(i, j, 0, 0);
                int pixelGreen = newImage(i, j, 0, 1);
                int pixelBlue = newImage(i, j, 0, 2);

                // Test red
//                BucketEntry redEntry = bucketData->redFinalBucket[i + j * width];

//                if (redEntry.isABucket && redEntry.id != getABucket(pixelRed))
//                    continue;

//                if (!redEntry.isABucket && redEntry.id != getBBucket(pixelRed))
//                    continue;

                // Test green
                BucketEntry greenEntry = bucketData->greenFinalBucket[idx];

                if (greenEntry.isABucket && greenEntry.id != getABucket(pixelGreen))
                    continue;

                if (!greenEntry.isABucket && greenEntry.id != getBBucket(pixelGreen))
                    continue;

                // Test blue
//                BucketEntry blueEntry = bucketData->blueFinalBucket[i + j * width];

//                if (blueEntry.isABucket && blueEntry.id != getABucket(pixelBlue))
//                    continue;

//                if (!blueEntry.isABucket && blueEntry.id != getBBucket(pixelBlue))
//                    continue;

                accRed[idx] += pixelRed;
                accGreen[idx] += pixelGreen;
                accBlue[idx] += pixelBlue;
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
    cimg::CImg<unsigned char> greenBucketImage(width, height, 1, 1, 0);
    cimg::CImgDisplay green_disp(width, height, "Green buckets");

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int idx = i + j * width;

            reconstructionImage(i, j, 0, 0) = (unsigned char) (accRed[i + j * width] / count[i + j * width]);
            reconstructionImage(i, j, 0, 1) = (unsigned char) (accGreen[i + j * width] / count[i + j * width]);
            reconstructionImage(i, j, 0, 2) = (unsigned char) (accBlue[i + j * width] / count[i + j * width]);

            int confidence = (int) (count[i + j * width] * ((float)256.0f / frames));
            confidence = std::min(confidence, 255);

            confidenceImage(i, j, 0, 0) = (unsigned char) confidence;
            confidenceImage(i, j, 0, 1) = (unsigned char) confidence;
            confidenceImage(i, j, 0, 2) = (unsigned char) confidence;

            if(count[idx] < 1)
            {
                confidenceImage(i, j, 0, 0) = 255;
                confidenceImage(i, j, 0, 1) = 0;
                confidenceImage(i, j, 0, 2) = 0;
            }

            BucketEntry greenEntry = bucketData->greenFinalBucket[i + j * width];
            unsigned char valueGreen = greenEntry.id * bucketSize;

            if(!greenEntry.isABucket)
            {
                valueGreen = 0;
//                valueGreen -= bucketSize / 2;

            }

            greenBucketImage(i, j, 0, 0) = valueGreen;

            if(greenEntry.diff < 1)
            {
                confidenceImage(i, j, 0, 0) = 0;
                confidenceImage(i, j, 0, 1) = 255;
                confidenceImage(i, j, 0, 2) = 0;

                std::cout << "Green offender - " << (int) greenEntry.id << " " << greenEntry.diff << " " << greenEntry.isABucket << std::endl;
            }


        }
        main_disp.render(reconstructionImage);
        main_disp.paint();

        aux_disp.render(confidenceImage);
        aux_disp.paint();

        green_disp.render(greenBucketImage);
        green_disp.paint();
    }

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
