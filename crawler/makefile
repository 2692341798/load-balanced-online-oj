contest_crawler:contest_crawler.cc
	g++ -o $@ $^ -I/usr/include/jsoncpp -I/opt/homebrew/include -L/opt/homebrew/lib -std=c++11 -ljsoncpp -lssl -lcrypto -lmysqlclient

test_crawler:test_crawler.cc
	g++ -o $@ $^ -I/usr/include/jsoncpp -I/opt/homebrew/include -L/opt/homebrew/lib -std=c++11 -ljsoncpp

luogu_crawler:luogu_crawler.cc
	g++ -o $@ $^ -I/usr/include/jsoncpp -I/opt/homebrew/include -L/opt/homebrew/lib -std=c++11 -ljsoncpp -lssl -lcrypto

.PHONY:clean
clean:
	rm -f contest_crawler test_crawler luogu_crawler
