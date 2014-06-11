#include "../include/Common.hpp"
#include "../include/FTPServer.hpp"
using namespace std;

/* Implementing the passive mode of the FTP protocol in terms of the data channel */ 


int main(int argc, char **argv)
{
	FTPServer ftpServer;
	int32_t connCtrlId;
	if (argc < 2) 
	{
		cerr << "ERROR, no port provided" << endl ;
		return 1;
    	}


	ftpServer.fillAddress(atoi(argv[1]));
	ftpServer.bindAddress();
	ftpServer.listenConn(5);
	connCtrlId = ftpServer.acceptCtrlConn();
	while(1)
	{	
		ftpServer.acceptCommand(connCtrlId);
	}	

	
	return 0; 

}
