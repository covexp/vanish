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

using namespace std;
using namespace cimg_library;

const string DEFAULT_DIRECTORY = "./input/";
const int DEFAULT_BUCKETSIZE = 8;
const int DEFAULT_BITDEPTH = 8;

int main(int argc, char *argv[])
{
	// Welcome message
	cout << "Vanish - Version 0.03 Alpha" << endl << endl;

	// Command line options
	opt::options_description desc("Allowed options");
	desc.add_options()
		("help", "show help message")
		("dir", opt::value<string>()->default_value(DEFAULT_DIRECTORY), "directory of input image sequence")
		("type", opt::value<string>()->default_value("png"), "file extension")
		("bucket", opt::value<int>()->default_value(DEFAULT_BUCKETSIZE), "bucket size")
		("depth", opt::value<int>()->default_value(DEFAULT_BITDEPTH), "channel bit depth")
		("refine", opt::value<int>()->default_value(0), "number of refinement steps")
		;

	opt::variables_map vm;

	try
	{
		opt::store(opt::parse_command_line(argc, argv, desc), vm);
		opt::notify(vm);
	}
	catch (const exception &e)
	{
		cerr << "Invalid command line parameter. Exiting." << endl;
		return 1;
	}

	if (vm.count("help"))
	{
		cout << desc << endl;
		return 1;
	}

	string inputDirectory = vm["dir"].as<string>();
	string fileExtension = vm["type"].as<string>();
	int bucketSize = vm["bucket"].as<int>();

	// Find image files
	fs::path imagePath(fs::initial_path());
	imagePath = fs::system_complete(fs::path(inputDirectory, fs::native));

	if (!fs::exists(imagePath))
	{
		cout << "Directory " << inputDirectory << " not found! Terminating." << endl;
		exit(EXIT_FAILURE);
	}

	vector<string> fileNames;

	fs::directory_iterator endItr;
	if (fs::is_directory(imagePath))
	{
		for (fs::directory_iterator itr(imagePath); itr != endItr; ++itr)
		{
			if (fs::is_regular_file(*itr))
			{
				if (itr->path().extension() == string(".") + fileExtension)
				{
					fileNames.push_back(itr->path().string());
				}
			}
		}
	}

	// Set up the parameters for the image processor
	ImageProcessor processor;
	processor.setBucketSize(bucketSize);
	processor.setFiles(fileNames);

	// Process the specified image sequence
	processor.processSequence();

	return 0;
}