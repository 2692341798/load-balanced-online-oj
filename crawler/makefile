contest_crawler:contest_crawler.cc
	g++ -o $@ $^ -I/opt/homebrew/include -L/opt/homebrew/lib -std=c++11 -ljsoncpp -lssl -lcrypto

test_crawler:test_crawler.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f contest_crawler test_crawler
