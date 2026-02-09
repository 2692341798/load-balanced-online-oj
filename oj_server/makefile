oj_server:oj_server.cc
	g++ -o $@ $^ -I./include -I/usr/include/jsoncpp -I/opt/homebrew/include -L/opt/homebrew/lib -std=c++11 -lpthread -lctemplate -ljsoncpp -lmysqlclient -lcrypto
.PHONY:clean
clean:
	rm -f oj_server