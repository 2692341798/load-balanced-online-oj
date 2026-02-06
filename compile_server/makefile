compile_server:compile_server.cc
	g++ -o $@ $^ -std=c++11 -I/usr/include/jsoncpp -ljsoncpp -lpthread
.PHONY:clean
clean:
	rm -f compile_server