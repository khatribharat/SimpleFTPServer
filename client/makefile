all:	FTPClient FTPClientAutomata main
	g++ obj/FTPClient.o obj/main.o obj/FTPClientAutomata.o -o bin/FTPClient
FTPClient:
	g++ -g -std=c++0x -c src/FTPClient.cpp -o obj/FTPClient.o
FTPClientAutomata:
	g++ -g -std=c++0x -c src/FTPClientAutomata.cpp -o obj/FTPClientAutomata.o
main:
	g++ -g -std=c++0x -c src/main.cpp -o obj/main.o
clean:
	rm -rf obj/*
	rm -rf bin/*

