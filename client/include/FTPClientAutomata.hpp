#ifndef CLIENTAUTOMATA_H
#define CLIENTAUTOMATA_H
#include "../include/Common.hpp"

// DEFINITION OF A DATA TYPE.
		enum state 	{	I,	/* Initial State */
					W,	/* Waiting State */
					S,	/* Success State */
					E,	/* Error State */
					F	/* Failure State */
				  };



class FTPClientAutomata
{
	private :
		enum state s;
	public :
		FTPClientAutomata();
		enum state move(int32_t *statusCode);
		enum state getCurrentState();


};
#endif
