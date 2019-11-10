// Vanish
// Remove transient objects from an image sequence

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include <cxxopts.hpp>

#include "image_processor.h"

namespace
{
    const std::string kDefaultDirectory = "./input/";
    const int kDefaultBucketSize = 8;
    const int kDefaultBitDepth = 8;
    const float kDefaultConfidenceLevel = 0.2f;

    const std::string kCmdHelp = "help";
    const std::string kCmdDirectory = "dir";
    const std::string kCmdType = "type";
    const std::string kCmdBucket = "bucket";
    const std::string kCmdDepth = "depth";
    const std::string kCmdSamples = "samples";
    const std::string kCmdConfidence = "conf";
}

int main(int argc, char* argv[])
{
    std::cout << "Vanish - Version 0.06 Alpha" << std::endl << std::endl;

    // Handle command line parameters
    cxxopts::Options options("Vanish", "Remove transient details from an image sequence.");
    options.add_options("default")
        (kCmdHelp, "show help message")
        (kCmdDirectory, "directory of input image sequence", cxxopts::value<std::string>())
        (kCmdType, "file extension", cxxopts::value<std::string>())
        (kCmdBucket, "bucket size", cxxopts::value<int>()->default_value(std::to_string(kDefaultBucketSize)))
        (kCmdDepth, "channel bit depth", cxxopts::value<int>())
        (kCmdSamples, "number of samples for bad frame detection", cxxopts::value<int>())
        (kCmdConfidence, "confidence level [0.0, 1.0]", cxxopts::value<float>()->default_value(std::to_string(kDefaultConfidenceLevel)));

    auto arguments = options.parse(argc, argv);

    if (arguments.count(kCmdDirectory) != 1)
    {
        std::cout << "Invalid command line arguments - Directory not specified." << std::endl;
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
    }

    if (arguments.count(kCmdType) != 1)
    {
        std::cout << "Invalid command line arguments - File type extension not specified." << std::endl;
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
    }

    std::string inputDirectory = arguments[kCmdDirectory].as<std::string>();
    std::string fileExtension = arguments[kCmdType].as<std::string>();

    int bucketSize = kDefaultBucketSize;
    if(arguments.count(kCmdBucket) == 1)
    {
        bucketSize = std::stoi(arguments[kCmdBucket].as<std::string>());
    }

    int bitDepth = kDefaultBitDepth;
    if(arguments.count(kCmdDepth) == 1)
    {
        bitDepth = std::stoi(arguments[kCmdDepth].as<std::string>());
    }

    float confLevel = kDefaultConfidenceLevel;
    if(arguments.count(kCmdConfidence) == 1)
    {
        confLevel = std::stof(arguments[kCmdConfidence].as<std::string>());
    }

    // Find image files
    std::vector<std::string> fileNames;
    std::filesystem::path imagePath(inputDirectory);

    // Check that directory exists
    if (!std::filesystem::exists(imagePath))
    {
        std::cerr << "Directory " << inputDirectory << " not found! Terminating." << std::endl;
        return EXIT_FAILURE;
    }

    std::filesystem::directory_iterator endItr;
    if (std::filesystem::is_directory(imagePath)) {
        for (std::filesystem::directory_iterator itr(imagePath); itr != endItr; ++itr) {
            if (std::filesystem::is_regular_file(*itr)) {
                if (itr->path().extension() == std::string(".") + fileExtension) {
                    fileNames.push_back(itr->path().string());
                }
            }
        }
    }

    for(auto fileName : fileNames)
    {
        std::cout << fileName << std::endl;
    }

    // Check that enough image files were found
    if (fileNames.size() < 2) {
        std::cerr << "Not enough files found in directory " << inputDirectory
            << "! Terminating." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Check the bucket size
    if (bucketSize < 1 || bucketSize > 128) {
        std::cerr << "Invalid bucket size. Using default value." << std::endl;
        bucketSize = kDefaultBucketSize;
    }

    // Set up the parameters for the image processor
    ImageProcessor processor;
    processor.setBucketSize(bucketSize);
    processor.setConfidenceLevel(confLevel);
    processor.setFiles(fileNames);

    // Process the specified image sequence
    processor.processSequence();

    return 0;
}
