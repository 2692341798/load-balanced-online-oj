# Tests Makefile

# Root directory of the project
ROOT_DIR = ..

# Directories
GTEST_DIR = lib/googletest/googletest
OJ_SERVER_DIR = $(ROOT_DIR)/oj_server
COMPILE_SERVER_DIR = $(ROOT_DIR)/compile_server
COMM_DIR = $(ROOT_DIR)/comm

# Flags
CXX = g++
CXXFLAGS += -g -Wall -Wextra -pthread -std=c++17
# Include paths: GTest, Project Roots, JSONCPP (system default)
CPPFLAGS += -isystem $(GTEST_DIR)/include -I$(ROOT_DIR) -I$(OJ_SERVER_DIR) -I$(COMPILE_SERVER_DIR) -I$(COMM_DIR) -I/usr/include/jsoncpp

# Platform specific flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CPPFLAGS += -I/opt/homebrew/include
	LDFLAGS += -L/opt/homebrew/lib
endif

# Libraries
# Note: ctemplate, jsoncpp, mysqlclient, crypto are required by the project
LIBS = -lpthread -ljsoncpp -lmysqlclient -lcrypto -lctemplate

# GTest Headers
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# GTest Source Files
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# Build GTest Libs
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^

# Test Targets
TEST_OJ_SERVER_OBJS = oj_server/test_oj_model.o oj_server/test_oj_control.o
TEST_COMPILE_SERVER_OBJS = compile_server/test_compiler.o compile_server/test_runner.o

# Main targets
all : test_oj_server test_compile_server stress_test security_test

# OJ Server Test
oj_server/test_oj_model.o : oj_server/test_oj_model.cc $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c oj_server/test_oj_model.cc -o $@

oj_server/test_oj_control.o : oj_server/test_oj_control.cc $(GTEST_HEADERS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c oj_server/test_oj_control.cc -o $@

test_oj_server : $(TEST_OJ_SERVER_OBJS) gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

# Compile Server Test
compile_server/test_compiler.o : compile_server/test_compiler.cc $(GTEST_HEADERS)
	mkdir -p compile_server
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c compile_server/test_compiler.cc -o $@

compile_server/test_runner.o : compile_server/test_runner.cc $(GTEST_HEADERS)
	mkdir -p compile_server
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c compile_server/test_runner.cc -o $@

test_compile_server : $(TEST_COMPILE_SERVER_OBJS) gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

# Stress Test
stress_test : stress_test.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) stress_test.cc -o $@ $(LDFLAGS) $(LIBS)

# Security Test
security_test : security_test.cc gtest_main.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

clean :
	rm -f test_oj_server test_compile_server stress_test security_test gtest.a gtest_main.a *.o oj_server/*.o compile_server/*.o
