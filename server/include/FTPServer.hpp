#ifndef FTPSERVER_H
#define FTPSERVER_H
#include <cstdint>
#include <vector>
#include <dirent.h>
#include "../include/Common.hpp"

#define R_BUFFER_SIZE 1024
#define S_BUFFER_SIZE 1024

using namespace std;
class FTPServer
{
	private:
		bool newConn;
		// Interface between the data module and the Control Module.
		bool dataComm;	

		// Used to store the current command and it's parameters.
		string command;
		string param;
		
		int32_t ctrlSockfd; 
		int32_t dataSockfd;
		struct sockaddr_in serverCtrlAddr, serverDataAddr;
		std::vector<int32_t> fdConnList;
		signed char rbuffer[R_BUFFER_SIZE];
		signed char sbuffer[S_BUFFER_SIZE];
		uint16_t dataPort;
	public :
		FTPServer();
		void fillAddress(uint16_t port);
		void bindAddress();
		//void bindAddrDataChannel();
		void listenConn(int32_t waitQueueLength);
		//void listenConnDataChannel(int32_t waitQueueLength);
		int32_t acceptCtrlConn();
		int32_t acceptDataConn();
		string serverReady();
		string serverDataReady();
		string commandParser(char* ftpCommand, int32_t ctrlfd);

		//############## CONTROL CHANNEL FUNCTIONS ##################
		string userCommand(string arg);
		string portCommand(string arg);
		string retrCommand(string arg);
		string storCommand(string arg);
		string cdCommand(string arg);
		
		string listCommand();
		string quitCommand(int32_t ctrlfd);
		string noopCommand();
		//#########################################################


		//########### DATA CHANNEL FUNCTIONS #####################
		void listData(string arg);
		void storData(string arg);
		void retrData(string arg);
		//########################################################
		
		void sendReply(int32_t sockfd, string reply);
		void sendData(string command, string param);
		void sendChar(int32_t sockfd, char ch);

		ssize_t receiveReply(int32_t sockfd);
		ssize_t receiveChar(int32_t sockfd);
		void acceptCommand(int32_t ctrlfd);

};
#endif
