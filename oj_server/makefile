UNAME_S := $(shell uname -s)

# Default flags (Linux)
INCLUDE_DIRS := -I./include -I/usr/include/jsoncpp
LIB_DIRS := 
LIBS := -lpthread -lctemplate -ljsoncpp -lmysqlclient -lcrypto

# macOS overrides
ifeq ($(UNAME_S),Darwin)
	INCLUDE_DIRS += -I/opt/homebrew/include
	LIB_DIRS += -L/opt/homebrew/lib
endif

oj_server:oj_server.cc
	g++ -o $@ $^ $(INCLUDE_DIRS) $(LIB_DIRS) -std=c++11 $(LIBS)

.PHONY:clean
clean:
	rm -f oj_server
