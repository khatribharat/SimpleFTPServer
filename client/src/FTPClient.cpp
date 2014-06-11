#include "../include/FTPClient.hpp"
#include <arpa/inet.h>
/*
FTP uses two TCP connections. 
(1) CONTROL CHANNEL ==> The client initiates the connection to the server at 
port 21 used for sending FTP commands and receiving FTP replies.

(2) DATA CHANNEL ==> (a) Active Mode : The server initiates the data channel.
(b) Passive Mode : The client initiates the data channel.
  
*/


FTPClient::FTPClient() 
{
	//############### CONTROL CHANNEL ##############
	this->ctrlfd = socket(AF_INET, SOCK_STREAM, 0);
	if ( ctrlfd < 0 )
	{	perror("sock ( Control Channel )::"); 
		exit(1);
	}
	//############### CONTROL CHANNEL #############
	
	//############### DATA CHANNEL ##############
	createDataSock();
	//############### DATA CHANNEL ##############

	// Clear the buffers.
	bzero(rbuffer, R_BUFFER_SIZE);
	bzero(sbuffer, S_BUFFER_SIZE);

	this->dataComm = false;
}


void FTPClient::createDataSock()
{
	this->datafd = socket(AF_INET, SOCK_STREAM, 0);
	if ( datafd < 0 )
	{	perror("sock (Data Channel )::"); 
		exit(1);
	}	
}

void FTPClient::SetServerAttr(char* hostname, uint16_t port)
{
	//############# CONTROL CHANNEL #################
	// Clear the buffers
	bzero(&(this->serverCtrlAddr), sizeof(this->serverCtrlAddr));

	(this->serverCtrlAddr).sin_family = AF_INET;
	(this->serverCtrlAddr).sin_addr.s_addr = inet_addr(hostname);  // argv[1] contains the I.P address
	(this->serverCtrlAddr).sin_port = htons(port); /* Port and I.P addresses need to be converted into network byte order before transmission. */
	//socklen_t saddr_len = sizeof(serverCtrlAddr); 
	//########### CONTROL CHANNEL ##################
	
	//############# DATA CHANNEL #################
	// Clear the buffers
	bzero(&(this->serverDataAddr), sizeof(this->serverDataAddr));

	(this->serverDataAddr).sin_family = AF_INET;
	(this->serverDataAddr).sin_addr.s_addr = inet_addr(hostname);  // argv[1] contains the I.P address
	(this->serverDataAddr).sin_port = htons(port - 1); /* Port and I.P addresses need to be converted into network byte order before transmission. */
	//socklen_t saddr_len = sizeof(serverDataAddr); 
	//########### DATA CHANNEL ##################

}

// interpretResponse() interprets the FTP reply and extracts the status code 
// appropriately for input to the FSM.
signed char* FTPClient::interpretResponse(signed char* response, ssize_t n)
{
	// Discard everything after the first EOL.   
	char* argE = strstr((char *)response,"\r\n");

	if ( argE == NULL )
	{
		std::cerr << "interpretCommmand :: Invalid FTP Reply\n";
		std::cerr << response;
	}
	char* temp = (char *)malloc(n + 1);
	
	// Copying 'response' into 'temp' because strtok modifies the passed string.
	strcpy(temp,(char *)response);
	char* pch  = strtok((char *)response," ");// The first token is the REPLY CODE.
	
	//#TODO Allocating no memory for this pointer. Corrupts the this pointer.
	int32_t *statusCode = (int32_t*) malloc(sizeof(int32_t)); 
	
	
	
	*statusCode = *pch - '0';
	(this->stateMachine).move(statusCode);
	
	return (signed char*)(temp + 4);	// The beginning of the response text.
	
}


ssize_t FTPClient::receiveChar(int32_t sockfd)
{
	ssize_t numBytes;
	bzero(this->rbuffer,R_BUFFER_SIZE);

	// Max number of bytes that can be read is R_BUFFER_SIZE - 1.	
	numBytes = read(sockfd,this->rbuffer,1);
    	if (numBytes < 0) 
        { 	
		perror("read ::");
	}
	rbuffer[numBytes] = '\0';
	return numBytes;
}



ssize_t FTPClient::receiveReply(int32_t sockfd)
{
	ssize_t numBytes;
	bzero(this->rbuffer,R_BUFFER_SIZE);

	// Max number of bytes that can be read is R_BUFFER_SIZE - 1.	
	numBytes = read(sockfd,this->rbuffer,R_BUFFER_SIZE - 1);
    	if (numBytes < 0) 
        { 	
		perror("read ::");
	}
	rbuffer[numBytes] = '\0';
	return numBytes;
}

enum protocolStatus FTPClient::checkCommandStatus()
{
	enum state status = (this->stateMachine).getCurrentState();

	if ( status != S )
	{
		if ( status == F or status == E or status == I)	// Encountered an error.	
			{
				return fail;
			}
	} 
	return success;

}

/* #TODO dataConnect doess not send a ftp Command, yet it is treated as one because 
   there is a welcome message from teh server prepended with a reply status code.
   REMOVE THIS ANAMOLY.
*/
void FTPClient::dataConnect()
{
	//std::cerr << "Establishing data channel\n";
	while ( connect(this->datafd,(struct sockaddr*)&(this->serverDataAddr), sizeof(this->serverDataAddr) ) < 0 )
	{
		perror("connect :");
	}
	//std::cerr << "Established the data connection\n";
}

/* #TODO FTPConnect does not send a ftp Command, yet it is treated as one because 
   there is a welcome message from teh server prepended with a reply status code.
    REMOVE THIS ANAMOLY.
*/
enum protocolStatus FTPClient::ctrlConnect()
{
	if ( connect(this->ctrlfd,(struct sockaddr*)&(this->serverCtrlAddr), sizeof(this->serverCtrlAddr) ) < 0 )
	{	perror("Connect :: ( Control Channel )"); 
		exit(1);
	}

	// #### Wait for the server to reply that the server is ready #####
	
	ssize_t numBytes = receiveReply(this->ctrlfd);	
	std::cerr << interpretResponse(this->rbuffer, numBytes) << std::endl;
	return checkCommandStatus();	
}


void FTPClient::writeChar(int32_t sockfd, char ch)
{
	bzero(this->sbuffer,S_BUFFER_SIZE);
	*sbuffer = ch;
	writeSock(this->datafd,1);	
}


void FTPClient::writeDataSock(std::string data)
{
	const char* tptr;
	tptr = data.c_str();
	
	bzero(this->sbuffer,S_BUFFER_SIZE);
	strcpy((char *)this->sbuffer, tptr);

	writeSock(this->datafd,data.length());	
}


ssize_t FTPClient::writeSock(int32_t sockfd, int32_t len)
{
	ssize_t numBytes;
	numBytes = write(sockfd, this->sbuffer, len);
	//###############################################################
	if (numBytes < 0) 
		{		
			perror("write ::");
		}

	return numBytes;
}

// Here the argument 'command' contains "the FTP command + it's assoc. param"
enum protocolStatus FTPClient::writeCtrlSock(std::string command)
{
	const char* tptr;
	tptr = command.c_str();
	
	bzero(this->sbuffer,S_BUFFER_SIZE);
	strcpy((char *)this->sbuffer, tptr);

	writeSock(this->ctrlfd,command.length());	
		
	ssize_t numBytes = receiveReply(this->ctrlfd);	
	std::cerr << interpretResponse(this->rbuffer, numBytes) << std::endl;
	return checkCommandStatus();	
}


enum protocolStatus FTPClient::sendUsername(std::string username)
{
	std::stringstream ss;
	ss << "USER " << username << "\r\n";
	// To convert stringstream to string in C++, use myStringStream.str().
	return writeCtrlSock(ss.str());
}


void FTPClient::llistCommand()
{
	DIR *dir;
	char* path = get_current_dir_name();
	dir = opendir (path);
	if (dir != NULL) 
	{
		struct dirent *ent;
		// print all the files and directories within directory 
  		while ((ent = readdir (dir)) != NULL) 
  		{
	    		printf("%s\n", ent->d_name);
  		}
  		
  		closedir (dir); 	
	}	
}


void FTPClient::lcdCommand(std::string param)
{
	ssize_t num = param.length();
	const char* tptr = param.c_str();
	char* path = new char(num + 1);
	strcpy(path,tptr);

	if ( chdir(path) < 0 )
	{
		perror("chdir ::");
	}

}

void FTPClient::executeCommand(std::string command, std::string param)
{
	if ( command == "LLIST" )
		llistCommand();
	else if ( command == "LCD" )
		lcdCommand(param);
}


void FTPClient::checkDataCommand(std::string command)
{
	
	if( command == "RETR" || 
	    command == "STOR" ||
	    command == "LIST" )
	{	
		this->dataComm = true;
	}
	else
		this->dataComm = false;
}


void FTPClient::listData()
{
	// Establish the data channel.
	dataConnect();
	ssize_t numBytes;
	numBytes = receiveReply(this->datafd);
	std::string s((char *)this->rbuffer);
	if ( ! (this->multipleCall) )
		std::cerr << this->rbuffer;
	else
	{
		std::replace( s.begin(), s.end(), '\n', ',');
		// Strip the last comma
		s = s.substr(0,s.size() - 1);

	}
	// Data connection closed by the server.
	close(this->datafd);
	// Create a new data socket for FUTURE.
	createDataSock();
	this->fileList = s;
}

void FTPClient::retrData(std::string param)
{
	char ch;
	char tch;
	ssize_t numBytes;
	std::ofstream out;
	std::string token;
	std::stringstream ss(param);
	// arg may be a ',' seperated list of arguments.	
	while ( getline(ss, token, ','/* DELIMITER */) )
	{
		// Establish Data Connection
		dataConnect();

		//########## Receive file character by character ############
		out.open(token.c_str(),std::ofstream::out);
		while ( numBytes = receiveChar(this->datafd) )
		{
			//#TODO strcpy(&ch,(char *)this->rbuffer);
			ch = *(this->rbuffer);
			out.put(ch);
		}
		//#######################################################
		out.close();
		// Close Data Connection (Indicates EOF )
		close(this->datafd);
		// Create a new data socket for FUTURE.
		createDataSock();

	}
}


void FTPClient::storData(std::string param)
{
	int32_t ch;
	std::ifstream in;
	std::string token, reply;
	std::stringstream ss(param);
	// arg may be a ',' seperated list of arguments.	
	while ( getline(ss, token, ','/* DELIMITER */) )
	{
		// Establish Data Connection
		dataConnect();

		//########## Send file character by character ############
		in.open(token.c_str(),std::ifstream::in);
		ch = in.get();
		while ( in.good() )
		{
			//#TODO if the file contains NULL character.
			writeChar(this->datafd, (char)ch);
			ch = in.get();
		}
		//#######################################################
		in.close();
		// Close Data Connection (Indicates EOF )
		close(this->datafd);
		// Create a new data socket for FUTURE.
		createDataSock();

	}

}

void FTPClient::handleDataComm(std::string command, std::string param)
{
	if ( command == "LIST" )
	{
		listData();
	}

	else if ( command == "STOR" )
	{	
		storData(param);
	}

	else if ( command == "RETR" )
	{
		retrData(param);
	}
}


std::string FTPClient::expandWildcard()
{
	std::stringstream ss;
	DIR *dir;
	char* path = get_current_dir_name();
	dir = opendir (path);
	if (dir != NULL) 
	{
		struct dirent *ent;
		// print all the files and directories within directory 
  		while ((ent = readdir (dir)) != NULL) 
  		{
	    		if ( strcmp(ent->d_name,".")  != 0 &&
			     strcmp(ent->d_name,"..") != 0 )
	
				ss << ent->d_name << ',';
  		}
  		
  		closedir (dir); 	
	}
	// The stringstream contains an extra comma, that we need to remove.
	std::string retval = ss.str();
	retval = retval.substr(0,retval.size() - 1);
	//std::cerr << "* expansion " << retval;
	return retval;
}



void FTPClient::sendCommand(std::string command, std::string param)
{
	enum protocolStatus state = fail;
	
	// Check if a command will require involvement of the data channel
	// and accordingly set the dataComm member of the class.
	checkDataCommand(command);
	
	std::stringstream ss;
	// If the commands pertain to local context, we need to intercept them.
	if ( command == "LLIST" || command == "LCD" )
	{
		executeCommand(command,param);	
	
	}

	else
	{
		if ( param == "" )
		{
			ss << command << "\r\n";
			state = writeCtrlSock(ss.str());
		}
		else
		{
			if ( param == "*" )
				param = expandWildcard();
			
			ss << command << " " << param << "\r\n";
			state = writeCtrlSock(ss.str());
		}
	}
	if ( this->dataComm && (state == success) )
	{
		// Now, We need to do something on the data channel based on the
		// command and the parameters just sent.
		//std::cerr << "On Data Channel\n";
		handleDataComm(command, param);

	}
}
