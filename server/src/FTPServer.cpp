#include "../include/FTPServer.hpp"
using namespace std;

/* This communication happens on the control channel. The communication is handled
   in "telnet strings". FTP commands are telnet strings terminated by the telnet 
   EOL character. The telnet was a character-by-character communication protocol
   using 7-bit ASCII codes echoing characters off the NVT. Here the 'telnet strings'  
   hold the same format but however are transferred one line at a time ( one line
   [COMMAND] all at once ) terminated by EOL which helps the end-device interpret the
   command sent and give and appropriate response 	
*/

/* 
   The 7-bit ASCII code is implemented using the signed char in C++ which uses the
   lower 7 bits for data and the MSB for sign and this representation can thus be used 
   for control codes with the MSB set to 1. The signed char representation can effectively
   represent the 128 ASCII code set used by the telnet protocol for transmission of
   DATA ( for the  control codes the telnet protocol uses the same with the MSB set to 1 )
*/

/*
	FTP Reply Format.
	A reply is defined to contain the 3-digit code, followed by Space
        <SP>, followed by one line of text (where some maximum line length
        has been specified), and terminated by the Telnet end-of-line
        code. It is transmiited as telnet strings.
	NOTE : 
 	1. On the other end, these are interpreted character-by-character as specified.	  
	by the telnet protocol.
 	2. Telnet EOL character is \r\n.
*/

FTPServer::FTPServer()
{
	
	//############### CONTROL CHANNEL ######################
	this->ctrlSockfd = socket(AF_INET,SOCK_STREAM,0);
	if ( this->ctrlSockfd < 0 )
	{	perror("socket ::");
		exit(1);
	}
	// Clear the address buffer
	bzero((char *)&(this->serverCtrlAddr),sizeof(this->serverCtrlAddr));
	//############# CONTROL CHANNEL ##########################

	//############## DATA CHANNEL ###########################
	this->dataSockfd = socket(AF_INET,SOCK_STREAM,0);
	if ( this->dataSockfd < 0 )
	{	perror("socket ::");
		exit(1);
	}
	// Clear the address buffer
	bzero((char *)&(this->serverDataAddr),sizeof(this->serverDataAddr));
	//############## DATA CHANNEL ###########################


	bzero(this->rbuffer, R_BUFFER_SIZE);
	bzero(this->sbuffer, S_BUFFER_SIZE);

	this->newConn = false;
	this->dataComm = false;
	this->command = "";
	this->param = "";

}

void FTPServer::fillAddress(uint16_t port)
{
	//############# CONTROL CHANNEL ##########################
	(this->serverCtrlAddr).sin_family = AF_INET;
	(this->serverCtrlAddr).sin_addr.s_addr = INADDR_ANY;
	(this->serverCtrlAddr).sin_port = htons(port);
	//############# CONTROL CHANNEL ##########################

	//############## DATA CHANNEL ###########################
	dataPort = htons(port - 1);
	(this->serverDataAddr).sin_family = AF_INET;
	(this->serverDataAddr).sin_addr.s_addr = INADDR_ANY;
	(this->serverDataAddr).sin_port = dataPort;
	//############## DATA CHANNEL ###########################

}

/*void FTPServer::bindAddrDataChannel()
{
	}
*/


void FTPServer::bindAddress()
{
	//############# CONTROL CHANNEL ##########################
	if ( bind(this->ctrlSockfd, (struct sockaddr *)&serverCtrlAddr, sizeof(this->serverCtrlAddr)) < 0 )
	{	perror("bind (Control Channel)::");
		exit(1);
	}
	//############# CONTROL CHANNEL ##########################

	//############## DATA CHANNEL ###########################
	if ( bind(this->dataSockfd, (struct sockaddr *)&serverDataAddr, sizeof(this->serverDataAddr)) < 0 )
	{	perror("bind (Data Channel)::");
		exit(1);
	}
	//############## DATA CHANNEL ###########################

	
}


void FTPServer::listenConn(int32_t waitQueueLength)
{
	//############# CONTROL CHANNEL ##########################
	listen(this->ctrlSockfd,waitQueueLength);
	//############# CONTROL CHANNEL ##########################

	//############## DATA CHANNEL ###########################
	listen(this->dataSockfd,waitQueueLength);
	//############## DATA CHANNEL ###########################

}

/*void FTPServer::listenConnDataChannel(int32_t waitQueueLength)
{
	}
*/

// #TODO See if using a pointer to a local resource in pair does not create problems.
string FTPServer::serverDataReady()
{
	string responseString("2XX Data Channel Ready\r\n");
	return responseString;
    
}

// #TODO See if using a pointer to a local resource in pair does not create problems.
string FTPServer::serverReady()
{
	string responseString("2XX Welcome to the FTP Server\r\n");
	return responseString;
    
}


void FTPServer::sendChar(int32_t sockfd, char ch)
{
	ssize_t numBytes;
	// Create a safe copy of temporary buffer;
	bzero(sbuffer,S_BUFFER_SIZE);
	*sbuffer = ch;
	
	numBytes = write(sockfd, this->sbuffer, 1);
	
	if (numBytes < 0) 
		{	
			cerr << "Write to " << sockfd << " failed\n";
			perror("write ::");
		}
}

void FTPServer::sendReply(int32_t sockfd, string reply)
{
	ssize_t numBytes;

	// Conversion from C++ string to CString.
	const char* tmpBuffer = reply.c_str();

	// Create a safe copy of temporary buffer;
	bzero(sbuffer,S_BUFFER_SIZE);
	strcpy((char *)sbuffer,tmpBuffer);
	
	numBytes = write(sockfd, this->sbuffer, reply.length());
	//###############################################################
	
	if (numBytes < 0) 
		{	
			cerr << "Write to " << sockfd << " failed\n";
			perror("write ::");
		}	
}


ssize_t FTPServer::receiveChar(int32_t sockfd)
{
	ssize_t numBytes;
	bzero(this->rbuffer, R_BUFFER_SIZE);
		
	numBytes = read(sockfd,this->rbuffer,1);
    	if (numBytes < 0)
        { 	
		perror("read ::");
	}
	
	(this->rbuffer)[numBytes] = '\0';
	return numBytes;
}

ssize_t FTPServer::receiveReply(int32_t sockfd)
{
	ssize_t numBytes;
	bzero(this->rbuffer, R_BUFFER_SIZE);
		
	numBytes = read(sockfd,this->rbuffer,R_BUFFER_SIZE - 1);
    	if (numBytes < 0)
        { 	
		perror("read ::");
	}
	
	(this->rbuffer)[numBytes] = '\0';
	return numBytes;
}



// acceptConn() return teh socket fd of the control channel.
int32_t FTPServer::acceptCtrlConn()
{
	int32_t newSockfd;	// New Control Connection
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	// Server is in listening mode on port 21 for ftp control channel.
	newSockfd = accept(this->ctrlSockfd, (struct sockaddr*)&clientAddr, &clientLen);
	if ( newSockfd < 0 )
		{	perror ("accept (Control Channel)::");
			exit(1);
		}
	// Else a successful TCP connection has been established on port 21.

	//#############  Notify that the server is ready ################
	sendReply(newSockfd, serverReady());
	//###############################################################
	(this->fdConnList).push_back(newSockfd);
	return newSockfd;
}

// acceptDataConn returns the socket fd of the data channel.
int32_t FTPServer::acceptDataConn()
{
	int32_t newSockfd;	// New Data Connection
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);

	
	std::cerr << "acceptDataConn :: Waiting on accept for Data Connection\n";
	// Server is in listening mode on port 20 for ftp data channel.
	newSockfd = accept(this->dataSockfd, (struct sockaddr*)&clientAddr, &clientLen);
	if ( newSockfd < 0 )
		{	perror ("accept (Data Channel)::");
			exit(1);
		}
	// Else a successful TCP connection has been established on port 20.

	return newSockfd;
}


string FTPServer::userCommand(string arg)
{
	stringstream ss;
	ss << "2XX " << arg << " is now authenticated to use the server\r\n";	
	return ss.str();
}

string FTPServer::portCommand(string arg)
{}

string FTPServer::retrCommand(string arg)
{
	
	ifstream in;
	bool fail = false;
	string token;
	stringstream ss(arg), ssReply;

	// arg may be a ',' seperated list of arguments.	
	while ( getline(ss, token, ','/* DELIMITER */) )
	{
		in.open(token.c_str(),ifstream::in);
		if ( in.fail() )
			{
				fail = true;
				break;	
			}
		in.close();
	}

	if ( fail )
	{
		ssReply << "5XX Some files are not readable\r\n";
	}
	else
	{
		ssReply << "2XX All files exist\r\n";
	}
	return ssReply.str();
}

string FTPServer::storCommand(string arg)
{
	stringstream ss;
	ss << "2XX " << arg << "OK\r\n";
	
	return ss.str();
}


string FTPServer::cdCommand(string arg)	
{
	stringstream ss;
	if ( chdir(arg.c_str()) < 0 )
	{
		perror("chdir :");
		ss << "5XX CD Command failed\r\n";
	}
	else
	{
		ss << "2XX OK pwd " << arg.c_str() << "\r\n";	 
	}
	return ss.str();
}

string FTPServer::listCommand()
{
	stringstream  ss;
	
	DIR *dir;
	char* path = get_current_dir_name();
	dir = opendir (path);
	if (dir != NULL) 
	{
  		closedir (dir); 
		ss << "2XX OK\r\n";
	}
	else
	{
		ss << ("5XX DIR command failed\r\n");
	}
	return ss.str();
}


string FTPServer::quitCommand(int32_t ctrlfd)
{
	close(ctrlfd);
	close(this->ctrlSockfd);
	close(this->dataSockfd);
	std::cerr << "Client with sockfd " << ctrlfd << " disconnected\n";
	exit(1);

}

string FTPServer::noopCommand()
{
	string responseString("NOOP :: Connection still alive\r\n");
	return responseString;
}


string FTPServer::commandParser(char* ftpCommand/*NULL TERMINATED STRING*/, int32_t ctrlfd)
{
	
	std::cerr << "commandParser :: " << ftpCommand;
	
	char *pch, *argS, *argE;
	this->newConn = false;	
	this->dataComm = false;
	this->command = "";
	this->param = "";
	
	argS = strchr (ftpCommand, ' '); //Beginning of argument.
	
	// Discard everything after the EOL character.
	argE = strstr (ftpCommand, "\r\n"); // Find the first occurence of \r\n. 	
	
	if ( argE == NULL )
	{
		// Invalid Command
		string s("4XX No EOL found\r\n");
		return s;
		
	}
	
	// Parsing requires identifying TERMINALS from the sentence whose 
	// grammar needs to be checked. The TERMINALS need to be space 
	// seperated and accordingly the FIRST and FOLLOW ( which are sets
	// of some TERMINALS ) are used match the TERMINALS found in the
	// sentence whith those contained in these sets.
	
	if ( argS == NULL )
	{
		if ( strcmp(ftpCommand,"LIST\r\n") == 0)
		{	
			this->dataComm = true;
			this->command = "LIST";
			return listCommand();
		}

		else if ( strcmp(ftpCommand,"QUIT\r\n") == 0 )
		{	
			return quitCommand(ctrlfd);
		}

		else if (strcmp(ftpCommand,"NOOP\r\n") == 0 )
		{	
			return noopCommand();
		}

		else
		{	
			string s("4XX Invalid command(1)\r\n");
			return s;
		}
	}
	else
	{
		char * tempCommand = (char *)malloc(strlen(ftpCommand + 1));
		// Because strtok tampers the string to be parsed. 
		strcpy(tempCommand, ftpCommand); 
		pch = strtok(tempCommand," ");	// Identify the TYPE of command.
		if ( strcmp(pch, "USER") == 0 )
		{
			string param(argS + 1,argE - argS - 1);
			this->newConn = true;
			return userCommand(param);	
		}

		else if ( strcmp(pch, "PORT") == 0 )
		{	
			string param(argS + 1,argE - argS - 1);
			//this->param = param;
			return portCommand(param);	
		}

		else if ( strcmp(pch,"RETR") == 0 )
		{
			this->dataComm = true;
			this->command = "RETR";
			string param(argS + 1,argE - argS - 1);
			this->param = param;
			return retrCommand(param);
		}

		else if ( strcmp(pch,"STOR") == 0)
		{	
			this->dataComm = true;	
			this->command = "STOR";
			string param(argS + 1,argE - argS - 1);
			this->param = param;
			return storCommand(param);
		}

		else if (strcmp(pch,"CD") == 0 )
		{	
			string param(argS + 1,argE - argS - 1);
			std::cerr << "In commandParser " << param.length();
			return cdCommand(param);
		}

		else
		{	
			string s("4XX Invalid Command(2)\r\n");
			return s;
		}

	}
	

}


void FTPServer::listData(string param)
{
	//std::cerr << "Accepting Data Connecetions\n";
	int32_t sockfd = acceptDataConn();	// Establish data connection.
	
	stringstream  ss;
	
	DIR *dir;
	char* path = get_current_dir_name();
	dir = opendir (path);
	if (dir != NULL) 
	{
		struct dirent *ent;
	  	// print all the files and directories within directory 
  		while ((ent = readdir (dir)) != NULL) 
  		{
	    		ss << ent->d_name << std::endl;
  		}
  		
  		closedir (dir); 
		
	}

	sendReply(sockfd, ss.str());
	close(sockfd);	
}

void FTPServer::storData(string arg)
{
	int32_t sockfd;
	char ch;
	ssize_t numBytes;
	ofstream out;
	string token;
	stringstream ss(arg);
	// arg may be a ',' seperated list of arguments.	
	while ( getline(ss, token, ','/* DELIMITER */) )
	{
		// Establish Data Connection
		sockfd = acceptDataConn();

		//########## Receive file character by character ############
		out.open(token.c_str(),ofstream::out);
		while ( numBytes = receiveChar(sockfd) )
		{
			//#TODO strcpy(&ch,(char *)this->rbuffer);
			ch = *(this->rbuffer);
			out.put(ch);
		}
		//#######################################################
		out.close();
		// Close Data Connection (Indicates EOF )
		close(sockfd);
	}
}



// This function handles communication for both single and multiple files.
void FTPServer::retrData(string arg)
{
	int32_t sockfd, ch;
	ifstream in;
	string token;
	stringstream ss(arg);
	// arg may be a ',' seperated list of arguments.	
	while ( getline(ss, token, ','/* DELIMITER */) )
	{
		// Establish Data Connection
		sockfd = acceptDataConn();

		//########## Send file character by character ############
		in.open(token.c_str(),ifstream::in);
		ch = in.get();
		while ( in.good() )
		{
			//#TODO if the file contains NULL character.
			sendChar(sockfd, ch);
			ch = in.get();
		}
		//#######################################################
		in.close();
		// Close Data Connection (Indicates EOF )
		close(sockfd);
	}

}


void FTPServer::sendData(string command, string param)
{
	if ( command == "LIST" )
		listData(param);
	else if ( command == "STOR" )
		storData(param);
	else if ( command == "RETR" )
		retrData(param);
	else
		return;
}

void FTPServer::acceptCommand(int32_t ctrlfd)
{
	ssize_t numBytes;

	numBytes = receiveReply(ctrlfd);
 
	sendReply(ctrlfd, commandParser((char *)this->rbuffer, ctrlfd));
	
	if ( dataComm )
	{
		sendData(this->command, this->param);
	}
	 
}

