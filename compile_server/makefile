compile_server:compile_server.cc
	g++ -o $@ $^ -std=c++11 -I/opt/homebrew/include -L/opt/homebrew/lib -ljsoncpp -lpthread
.PHONY:clean
clean:
	rm -f compile_server