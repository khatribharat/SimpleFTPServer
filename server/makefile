all:	FTPServer main
	g++ obj/FTPServer.o obj/main.o -o bin/FTPServer
FTPServer:
	g++ -g -std=c++0x -c src/FTPServer.cpp -o obj/FTPServer.o
main:
	g++ -g -std=c++0x -c src/main.cpp -o obj/main.o
clean:
	rm -rf obj/*
	rm -rf bin/*

