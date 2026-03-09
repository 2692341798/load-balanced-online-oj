CXX = g++
CXXFLAGS = -std=c++11 -Wall -g
INCLUDES = -I/usr/local/include -I/opt/homebrew/include -I/usr/include
LIBS = -L/usr/local/lib -L/opt/homebrew/lib -lhiredis

all: test_redis_queue

test_redis_queue: test_redis_queue.cc ../../crawler/redis_queue.hpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LIBS)

clean:
	rm -f test_redis_queue
