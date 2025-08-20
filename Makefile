CXX=g++
CXXFLAGS = -O3 -MMD -MP -Wall -c -m64 -std=c++23 -pthread

all: main

debug: CXXFLAGS += -g
debug: main

#  -lncurses
main: pngTest.o pngSimple.o pngIO.o pngHelper.o matrix.o arguments.o editDistance.o
	$(CXX) -o pcell pngTest.o pngSimple.o pngIO.o pngHelper.o matrix.o arguments.o editDistance.o -lpng

# 
#       -c      only compile without performing the link operation
#       -g      debugging-information (for gdb)
#       -g1 -g2 -g3     more debugging information, with g3 the highest. -g0 turns debug infos off. -g2 is equivalent to -g
#       -O      disable optimization (for debugging)
#       -O1 -O2 -O3     Optimization levels with O3 the highest. -O0 means no optimization which is equivalent to -O
#       -Wall   Warning all
#       -m64    creates code for 64 bit ABI
#       -MMD
#       -MP     This option instructs CPP to add a phony target for each dependency other than the main file, causing each to depend on nothing. These dummy rules work around errors make gives if you remove header files without updating the Makefile to match. 
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^
	
clean:
	rm pcell *.o
