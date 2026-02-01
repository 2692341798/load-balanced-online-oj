contest_crawler:contest_crawler.cc
	g++ -o $@ $^ -I/opt/homebrew/include -L/opt/homebrew/lib -std=c++11 -ljsoncpp -lssl -lcrypto -lmysqlclient

test_crawler:test_crawler.cc
	g++ -o $@ $^ -I/opt/homebrew/include -L/opt/homebrew/lib -std=c++11 -ljsoncpp


.PHONY:clean
clean:
	rm -f contest_crawler test_crawler