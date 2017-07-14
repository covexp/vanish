// Vanish
// Remove transient objects from an image sequence

#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include "../../cimg/CImg.h"

#include "imageProcessor.h"

namespace opt = boost::program_options;
namespace fs = boost::filesystem;
namespace cimg = cimg_library;

const std::string DEFAULT_DIRECTORY = "./input/";
const int DEFAULT_BUCKETSIZE = 8;
const int DEFAULT_BITDEPTH = 8;

int main(int argc, char *argv[])
{
	// Welcome message
	std::cout << "Vanish - Version 0.04 Alpha" << std::endl << std::endl;

	// Command line options
	opt::options_description desc("Allowed options");
	desc.add_options()
		("help", "show help message")
		("dir", opt::value<std::string>()->default_value(DEFAULT_DIRECTORY), "directory of input image sequence")
		("type", opt::value<std::string>()->default_value("png"), "file extension")
		("bucket", opt::value<int>()->default_value(DEFAULT_BUCKETSIZE), "bucket size")
		("depth", opt::value<int>()->default_value(DEFAULT_BITDEPTH), "channel bit depth")
		("refine", opt::value<int>()->default_value(0), "number of refinement steps")
		("samples", opt::value<int>()->default_value(64), "number of samples for bad frame detection")
		;

	opt::variables_map vm;

	try
	{
		opt::store(opt::parse_command_line(argc, argv, desc), vm);
		opt::notify(vm);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Invalid command line parameter. Exiting." << std::endl;
		return 1;
	}

	if (vm.count("help"))
	{
		std::cout << desc << std::endl;
		return 1;
	}

	std::string inputDirectory = vm["dir"].as<std::string>();
	std::string fileExtension = vm["type"].as<std::string>();
	int bucketSize = vm["bucket"].as<int>();

	// Find image files
	fs::path imagePath(fs::initial_path());
	imagePath = fs::system_complete(fs::path(inputDirectory, fs::native));

	// Check that directory exists
	if (!fs::exists(imagePath))
	{
		std::cerr << "Directory " << inputDirectory << " not found! Terminating." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::vector<std::string> fileNames;

	fs::directory_iterator endItr;
	if (fs::is_directory(imagePath))
	{
		for (fs::directory_iterator itr(imagePath); itr != endItr; ++itr)
		{
			if (fs::is_regular_file(*itr))
			{
				if (itr->path().extension() == std::string(".") + fileExtension)
				{
					fileNames.push_back(itr->path().string());
				}
			}
		}
	}

	// Check that enough image files were found
	if (fileNames.size() < 2)
	{
		std::cerr << "Not enough files found in directory " << inputDirectory << "! Terminating." << std::endl;
		exit(EXIT_FAILURE);
	}

	// Set up the parameters for the image processor
	ImageProcessor processor;
	processor.setBucketSize(bucketSize);
	processor.setFiles(fileNames);

	// Process the specified image sequence
	processor.processSequence();

	return 0;
}