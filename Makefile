build:
	g++ -std=c++11 -o bin/customshell cmd/terminal/main.cpp

run: build	
	./bin/customshell