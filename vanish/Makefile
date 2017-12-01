# Linux makefile for vanish

vanish: image_processor.o vanish.o
	g++ -fopenmp -std=c++11 -o vanish image_processor.o vanish.o -lstdc++ -lm -lpthread -lX11 -lboost_system -lboost_filesystem -lboost_program_options

image_processor.o: image_processor.cpp image_processor.h
	g++ -fopenmp -std=c++11 -c image_processor.cpp

vanish.o: vanish.cpp
	g++ -fopenmp -std=c++11 -c vanish.cpp

clean:
	rm vanish image_processor.o vanish.o

# all:
#		g++ -std=c++11 bucketData.cpp imageProcessor.cpp vanish.cpp -lstdc++ -lm -lpthread -lX11 -lboost_system -lboost_filesystem -lboost_program_options -o vanish
	
