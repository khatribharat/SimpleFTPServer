#include "../include/FTPClientAutomata.hpp"

FTPClientAutomata::FTPClientAutomata()
{
	this->s = I;
};

enum state FTPClientAutomata::move(int32_t *statusCode)
{
	int32_t status = *statusCode;
	//#TODO extract status from the reply
	switch(status)
	{
		case 1:
			this->s = W;
			break;
		case 2:
			this->s = S;
			break;
		case 3:
			this->s = I;
			break;
		case 4:
			this->s = F;
			break;
		case 5:
			this->s = F;
			break;

		default:
			std::cerr << "Unknown Reply Code " << status << std::endl;
			break;

	}
	return this->s;
}


enum state FTPClientAutomata::getCurrentState()
{
	return this->s;
}
