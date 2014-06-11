#include "../include/FTPClient.hpp"
#include <sstream>
#include <limits> // for numeric_limits

using namespace std;
int main(int argc, char *argv[])
{
	int n;
	string username;
	string command, param;	// For the FTP Terminal interpreter.
	
	if (argc < 3) 
	{
		std::cerr << "usage " << argv[0] << " hostname port\n";
		exit(0);
	}
	
	FTPClient ftpClient;
	ftpClient.SetServerAttr(argv[1],atoi(argv[2]));
	//#####  Wait for the server to reply that the server is ready ###########
	if ( ftpClient.ctrlConnect() != success )
	{	
		std::cerr << "Error in establishing Control Connection at port 21\n";
		exit(1);
	}
   
	

	// Now the client needs to pass the username. Via the user command.
	// We need to show the ftp user prompt at the client NVT.
	// #TODO Grab the server name from the response of FTPConnect.
	
	cout << "ftp> USER : ";
	cin >> username;
	if ( ftpClient.sendUsername(username) != success )
	{	std::cerr << "Invalid username. Try Again\n";
		exit(1);
	}

	//##### FTP TERMINAL COMMAND INTERPRETER.
	// The following getline function is globally defined in C++-string
	// library and works with c++ strings. Another function with the name
	// also exists in the iostream library under the istream namespace and
	// works with C-like strings.
	
	//getline(cin,command,' ');
	
	
	cout << "ftp>";
	cin >> command;

	while ( command != "EXIT" )
	{
		ftpClient.multipleCall = false;

		if ( command == "GET" || command == "get" )
		{
			command = "RETR";
			cin >> param;
			//cout << "Parameter is " << param << endl;
			if ( param.find_first_of(", ") != string::npos )
			{	
				std::cerr << "Usage :: GET/get <filename>\n";
				continue;

			}
			ftpClient.sendCommand(command,param);
		}
		else if ( command == "PUT" || command == "put" )
		{
			command = "STOR";
			cin >> param;
			//cout << "Parameter is " << param << endl;
			if ( param.find_first_of(", ") != string::npos )
			{	
				std::cerr << "Usage :: PUT/put <filename>\n";
				continue;
			}

			ftpClient.sendCommand(command,param);
		}

		else if ( command == "DIR" || command == "dir" )
		{
			command = "LIST";
			ftpClient.sendCommand(command,"");
		
		}

		else if ( command == "LDIR" || command == "ldir" )
		{
			command = "LLIST";
			ftpClient.sendCommand(command,"");
		
		}

		else if ( command == "CD" || command == "cd" )
		{
			command = "CD";
			cin >> param;
			//cout << "Parameter is " << param << endl;
			ftpClient.sendCommand(command,param);
		
		}
		
		else if ( command == "LCD" || command == "lcd" )
		{
			command = "LCD";
			cin >> param;
			//cout << "Parameter is " << param << endl;
			ftpClient.sendCommand(command,param);
		
		}

		else if ( command == "MGET" || command == "mget" )
		{
			command = "RETR";
			cin >> param;
			if ( param == "*" )
			{
			// First we need to get the list from the server.
			// getList gets a csv list from the server.
			 	ftpClient.multipleCall = true;
				ftpClient.sendCommand("LIST","");
				param = ftpClient.fileList;

			}
			//cout << "Parameter is " << param << endl;
			ftpClient.sendCommand(command,param);

		}

		else if ( command == "MPUT" || command == "mput" )
		{
			command = "STOR";
			cin >> param;
			//cout << "Parameter is " << param << endl;
			ftpClient.sendCommand(command,param);
		}
		/*
		if ( command == "NOOP" || command == "QUIT" )
			ftpClient.sendCommand(command,"");
		else if ( command == "PORT" ||
			  command == "STOR" || 
			  command == "RETR" ||
			  command == "LIST" )
		{
			// The istream still contains some parameter that
			// is required with these commands. Hence we extract it.
			// If the user presses an enter, cin will prompt the use
			// to enter the parameter continously until it finds 
			// a valid character other than a newline. HENCE NO
			// ERROR RECIVERY IS NEEDED HERE TO GET THE PARAMATER
			// FROM THE USER.
			cin >> param;
			cout << "Parameter is " << param << endl;
			ftpClient.sendCommand(command,param);

		}*/
		else
		{
			cout << "Invalid command. Type \"EXIT\" to exit." << endl;
		
		}
		// IMPORTANT :: Before we reach here, the input stream is 
		// cleared before we take our NEXT INPUT.
		// The followint line just does that. "ignore as many 
		// characters as necessary until you ignore a newline, 
		// then stop"
		//cin.ignore(numeric_limits<streamsize>::max(),'\n');
		cin.ignore(numeric_limits<streamsize>::max(),'\n');
		cout << "ftp>";
		cin >> command;
	}

    	//######################################

   return 0;
}
