UNAME_S := $(shell uname -s)

# Default flags (Linux)
INCLUDE_DIRS := -I/usr/include/jsoncpp
LIB_DIRS := 
# Common libs for all targets
LIBS_COMMON := -ljsoncpp
LIBS_SSL := -lssl -lcrypto
LIBS_MYSQL := -lmysqlclient

# macOS overrides
ifeq ($(UNAME_S),Darwin)
	INCLUDE_DIRS += -I/opt/homebrew/include
	LIB_DIRS += -L/opt/homebrew/lib
endif

.PHONY: all clean

all: contest_crawler test_crawler luogu_crawler

contest_crawler:contest_crawler.cc
	g++ -o $@ $^ $(INCLUDE_DIRS) $(LIB_DIRS) -std=c++11 $(LIBS_COMMON) $(LIBS_SSL) $(LIBS_MYSQL)

test_crawler:test_crawler.cc
	g++ -o $@ $^ $(INCLUDE_DIRS) $(LIB_DIRS) -std=c++11 $(LIBS_COMMON)

luogu_crawler:luogu_crawler.cc
	g++ -o $@ $^ $(INCLUDE_DIRS) $(LIB_DIRS) -std=c++11 $(LIBS_COMMON) $(LIBS_SSL)

clean:
	rm -f contest_crawler test_crawler luogu_crawler
