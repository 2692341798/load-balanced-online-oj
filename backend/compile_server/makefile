UNAME_S := $(shell uname -s)

# Default flags (Linux)
INCLUDE_DIRS := -I/usr/include/jsoncpp
LIB_DIRS := 
LIBS := -ljsoncpp -lpthread

# macOS overrides
ifeq ($(UNAME_S),Darwin)
	INCLUDE_DIRS += -I/opt/homebrew/include
	LIB_DIRS += -L/opt/homebrew/lib
endif

compile_server:compile_server.cc
	g++ -o $@ $^ -std=c++11 $(INCLUDE_DIRS) $(LIB_DIRS) $(LIBS)

.PHONY:clean
clean:
	rm -f compile_server
