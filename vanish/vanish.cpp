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

const int MINVAL = 0;
const int MAXVAL = 255;
const int WIDTH = 480;
const int HEIGHT = 480;
const int FRAMES = 67;
const int BUCKETSIZE = 8;
const int BUCKETS = (MAXVAL + 1) / BUCKETSIZE;

const int DEFAULT_FRAMES = 50;
const int DEFAULT_BUCKETSIZE = 8;

const string DEFAULT_DIRECTORY = "input";
const string FILESTEM = "picpick";
const string FILETYPE = ".png";

int main(int argc, char *argv[])
{
	// Welcome message
	cout << "Vanish - Version 0.02" << endl << endl;

	// Command line options
	opt::options_description desc("Allowed options");
	desc.add_options()
		("help", "show help message")
		("dir", opt::value<string>()->default_value("./input/"), "directory of input image sequence")
		("type", opt::value<string>()->default_value("png"), "file extension")
		("frames", opt::value<int>()->default_value(50), "number of frames")
		("bucket", opt::value<int>()->default_value(8), "bucket size")
		("depth", opt::value<int>()->default_value(8), "channel bit depth")
		("w", opt::value<int>(), "image width")
		("h", opt::value<int>(), "image height")
		;

	opt::variables_map vm;
	opt::store(opt::parse_command_line(argc, argv, desc), vm);
	opt::notify(vm);

	if (vm.count("help"))
	{
		cout << desc << endl;
		return 1;
	}

	string inputDirectory = vm["dir"].as<string>();
	string fileExtension = vm["type"].as<string>();
	int numFrames = vm["frames"].as<int>();

	// Find image files
	fs::path imagePath(fs::initial_path());
	imagePath = fs::system_complete(fs::path(inputDirectory, fs::native));

	if (!fs::exists(imagePath))
	{
		cout << "Directory " << inputDirectory << " not found! Terminating." << endl;
		return 1;
	}

	vector<string> fileNames;

	fs::directory_iterator endItr;
	if (fs::is_directory(imagePath))
	{
		for (fs::directory_iterator itr(imagePath); itr != endItr; ++itr)
		{
			if (fs::is_regular_file(*itr))
			{
				if (itr->path().extension() == string(".") + "png")
				{
//					cout << *itr << endl;
					fileNames.push_back(itr->path().string());
				}
			}
		}
	}

	// Set up the parameters for the image processor
	ImageProcessor *processor = new ImageProcessor();
	processor->setFrames(numFrames);
	processor->setFiles(fileNames);

	// Process the specified image sequence
	processor->processSequence();

	return 0;
}