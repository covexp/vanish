// Vanish
// Remove transient objects from an image sequence

#include <iostream>
#include <string>
#include <vector>
#include <boost/program_options.hpp>
#include "../../cimg/CImg.h"

#include "imageProcessor.h"

namespace opt = boost::program_options;

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
		("dir", opt::value<string>(), "directory of input image sequence")
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

	string inputDirectory;

	if (vm.count("dir"))
	{
		cout << "Input directory was: " << vm["dir"].as<string>() << endl;
		inputDirectory = vm["dir"].as<string>();
	}
	else
	{
		cout << "Directory was not set." << endl;
		inputDirectory = DEFAULT_DIRECTORY;
	}

	int numFrames = vm["frames"].as<int>();

	// Set up the parameters for the image processor
	ImageProcessor *processor = new ImageProcessor();
	processor->setInputDirectory(inputDirectory);
	processor->setFrames(numFrames);

	// Process the specified image sequence
	processor->processSequence();

	return 0;
}