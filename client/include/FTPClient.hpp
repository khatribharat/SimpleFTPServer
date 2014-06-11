#ifndef FTPCLIENT_H
#define FTPCLIENT_H
#include "Common.hpp"
#include <dirent.h>
#include <algorithm>
#include "FTPClientAutomata.hpp"
#define S_BUFFER_SIZE 1024
#define R_BUFFER_SIZE 1024

enum protocolStatus {	success,
			fail,
			intermed
		    };

class FTPClient
{
	private:
		bool dataComm;
		int32_t ctrlfd, datafd;
		struct sockaddr_in serverCtrlAddr, serverDataAddr;
		signed char sbuffer[S_BUFFER_SIZE];
		signed char rbuffer[R_BUFFER_SIZE];
		uint16_t dataPort;
		class FTPClientAutomata stateMachine;

	public:
		
		std::string fileList;
		bool multipleCall;
		
		FTPClient();
		void createDataSock();
		void SetServerAttr(char* hostname, uint16_t port);


		enum protocolStatus ctrlConnect();
		void dataConnect();		


		signed char* interpretResponse(signed char* response, ssize_t n);
		ssize_t receiveReply(int32_t sockfd);
		//#TODO This function was needed for accepting a single
		// character at a time, putting 1 for receiver BUFFER SIZE
		// in the read function, otherwise it was reading more than
		// a single character.
		ssize_t receiveChar(int32_t sockfd);

		enum protocolStatus sendUsername(std::string username);
		enum protocolStatus checkCommandStatus();
		void sendCommand(std::string command, std::string param);
		
		enum protocolStatus writeCtrlSock(std::string command);
		void writeDataSock(std::string data);
		ssize_t writeSock(int32_t sockfd, int32_t len);
		void writeChar(int32_t sockfd, char ch);

		void checkDataCommand(std::string command);
		void handleDataComm(std::string command, std::string param);

		void executeCommand(std::string command, std::string param);
		void llistCommand();
		void lcdCommand(std::string param);

		//############# DATA HANDLING FUNCTIONS ###########
		void listData();
		void storData(std::string param);
		void retrData(std::string param);
		//#################################################
		std::string expandWildcard();

};
#endif
