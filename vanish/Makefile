# Linux makefile for vanish

vanish: imageProcessor.o vanish.o
	g++ -std=c++11 -o vanish imageProcessor.o vanish.o -lstdc++ -lm -lpthread -lX11 -lboost_system -lboost_filesystem -lboost_program_options

imageProcessor.o: imageProcessor.cpp imageProcessor.h
	g++ -std=c++11 -c imageProcessor.cpp

vanish.o: vanish.cpp
	g++ -std=c++11 -c vanish.cpp

clean:
	rm vanish imageProcessor.o vanish.o

# all:
#		g++ -std=c++11 bucketData.cpp imageProcessor.cpp vanish.cpp -lstdc++ -lm -lpthread -lX11 -lboost_system -lboost_filesystem -lboost_program_options -o vanish
	
