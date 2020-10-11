/*
	This file is part of Vajolet.

    Vajolet is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vajolet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vajolet.  If not, see <http://www.gnu.org/licenses/>
*/

#include <csignal>
#include <iostream>

#include "book.h"

#include "libchess.h"

#include "spsa.h"

void signalHandler(int signum)
{
	exit(signum);
}

/*!	\brief	print the startup information
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
static void printStartInfo(void)
{
	std::cout <<"Vajolet tuner"<< std::endl;
}

int main() {
	
	signal(SIGINT, signalHandler); 
#ifdef SIGBREAK	
	signal(SIGBREAK, signalHandler); 
#endif
#ifdef SIGHUP		
	signal(SIGHUP, signalHandler);  
#endif

	printStartInfo();
	//----------------------------------
	//	init global data
	//----------------------------------
	libChessInit();
	
	Book book("book.pgn");
	/*while(true) {
		auto moves = b.getLine();
		for(const auto& m: moves) {
			std::cout<<UciOutput::displayUci(m, false)<<" ";
		}
		std::cout<<std::endl;
	}*/
	SPSA spsa(book);
	spsa.run();

	return 0;
}